// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

#include <Mutex.h>
#include <UniqueLock.h>
#include <String.h>
#include <fs/ELF.h>
#include <fs/Initramfs.h>
#include <kernel/Compiler.h>
#include <kernel/ExceptionManager.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <libs/Math.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

// The pointer to the init and kthreadd task.
Task* Task::_init = nullptr;
Task* Task::_kthreadd = nullptr;

// PID starts at 0 (idle task)
uint32_t Task::_next_pid = 0;


Task* Task::get_by_pid(const pid_t pid) {
  if (pid == 1) {
    return Task::_init;
  } else if (pid == 2) {
    return Task::_kthreadd;
  }

  // Process tree BFS
  List<Task*> q;
  q.push_back(Task::_init);

  while (!q.empty()) {
    size_t size = q.size();

    for (size_t i = 0; i < size; i++) {
      Task* task = q.front();
      q.pop_front();

      auto result = task->_active_children.find_if([pid](auto t) {
        return t->_pid == pid;
      });

      if (result) {
        return *result;
      }

      // Push all children onto the queue.
      task->_active_children.for_each([&q](auto t) {
        q.push_back(t);
      });
    }
  }
  
  return nullptr;
}


int Task::do_fork() {
  // Duplicate task
  save_context();

  size_t ret = 0;
  size_t parent_usp;
  asm volatile("mrs %0, sp_el0" : "=r" (parent_usp));

  size_t user_sp_offset = _ustack_page.offset_of(parent_usp);
  size_t kernel_sp_offset = _kstack_page.offset_of(_context.sp);
  size_t trap_frame_offset = _kstack_page.offset_of(_trap_frame);


  // Duplicate task.
  auto task = make_unique<Task>(/*parent=*/this, _entry_point, _name);
  Task* child = task.get();

  if (!task) {
    printk("do_fork: task class allocation failed (out of memory).\n");
    ret = -1;
    goto out;
  }

  if (!task->_kstack_page.get()) {
    printk("do_fork: kernel stack allocation failed (out of memory).\n");
    ret = -1;
    goto out;
  }

  if (!task->_ustack_page.get()) {
    printk("do_fork: user stack allocation failed (out of memory).\n");
    ret = -1;
    goto out;
  }

  // Enqueue the child task.
  TaskScheduler::get_instance().enqueue_task(move(task));

  // Copy kernel/user stack content
  child->_kstack_page.copy_from(_kstack_page);
  child->_ustack_page.copy_from(_ustack_page);
  
  /* ------ You can safely modify the child now ------ */

  // Set parent's fork() return value to child's pid.
  ret = child->_pid;

  // Copy child's CPU context.
  child->_context = _context;
  child->_context.lr = reinterpret_cast<uint64_t>(&&out);
  child->_context.sp = child->_kstack_page.add_offset(kernel_sp_offset);

  // Calculate child's trap frame.
  // When the child returns from kernel mode to user mode,
  // child->_trap_frame->sp_el0 will be restored to the child
  // and used as the user mode SP.
  child->_trap_frame = child->_kstack_page.add_offset<TrapFrame*>(trap_frame_offset);
  child->_trap_frame->sp_el0 = child->_ustack_page.add_offset(user_sp_offset);

out:
  //printf("pid: %d, ret = %d\n", Task::get_current()._pid, ret);
  return ret;
}


int Task::do_exec(const char* name, const char* const _argv[]) {
  // Update task name
  strncpy(_name, name, TASK_NAME_MAX_LEN - 1);

  // Construct the argv chain on the user stack.
  size_t user_sp = construct_argv_chain(_argv);
  size_t kernel_sp = _kstack_page.end();

  // Reset the stack pointer.
  _context.sp = user_sp;

  kfree(_elf_dest);

  // Load the specified file from the filesystem.
  ELF elf(Initramfs::get_instance().read(name));
  _elf_dest = kmalloc(elf.get_size() + 0x1000);
  void* dest = reinterpret_cast<char*>(_elf_dest) + 0x1000 - 0x10;

  if (!elf.is_valid()) {
    goto failed;
  }

  if (!_elf_dest) {
    goto failed;
  }

  printk("loading ELF at 0x%x\n", dest);
  elf.load_at(dest);

  // Jump to the entry point.
  printk("executing new program: %s, _kstack_page = 0x%x, _ustack_page = 0x%x\n",
         _name, _kstack_page, _ustack_page);

  ExceptionManager::get_instance()
    .downgrade_exception_level(0,
                               elf.get_entry_point(dest),
                               reinterpret_cast<void*>(kernel_sp),
                               reinterpret_cast<void*>(user_sp));
failed:
  printf("exec failed: pid = %d [%s]\n", _pid, _name);
  return -1;
}


int Task::do_wait(int* wstatus) {
  if (!get_children_count()) {
    return -1;
  }

  // Suspends execution of the calling thread until
  // one of its children terminates.
  while (_terminated_children.empty()) {
    TaskScheduler::get_instance().schedule();
  }

  auto& child = _terminated_children.front();

  if (wstatus) {
    *wstatus = child->_error_code;
  }

  int ret = child->_pid;
  _terminated_children.pop_front();
  return ret;
}


[[noreturn]] void Task::do_exit(int error_code) {
  // Terminate the current task.
  _state = Task::State::TERMINATED;
  _error_code = error_code;

  kfree(_elf_dest);

  Mutex m;
  m.lock();
  auto& sched = TaskScheduler::get_instance();
  _parent->_active_children.remove(this);
  _parent->_terminated_children.push_back(sched.remove_task(*this));
  m.unlock();

  sched.schedule();
  Kernel::panic("sys_exit: returned from sched.\n");
}


long Task::do_kill(pid_t pid, Signal signal) {
  // Send a signal to `pid`.
  if (auto task = Task::get_by_pid(pid)) {
    task->_pending_signals.push_back(signal);
    return 0;
  }

  printk("sys_kill: failed (pid %d not found)\n", pid);
  return -1;
}


int Task::do_signal(int signal, void (*handler)()) {
  if (unlikely(!is_signal_valid(signal))) {
    printk("do_signal: failed (signal=0x%x is invalid)\n", signal);
    return -1;
  }

  _custom_signal_handlers[signal] = handler;
  return 0;  // TODO: return previous handler's error code.
}


size_t Task::construct_argv_chain(const char* const _argv[]) {
  // Construct the argv chain.
  size_t user_sp = _ustack_page.end() - 0x10;
  printk("user_sp = 0x%x\n", user_sp);

  int argc = 0;

  UniquePtr<String[]> argv;
  UniquePtr<char*[]> addresses;

  char** new_argv;
  char** new_argv_data = nullptr;

  if (!_argv) {
    goto out;
  }

  for (const char* s = _argv[0]; s; s = _argv[++argc]);
  printk("argc = %d\n", argc);


  argv = make_unique<String[]>(argc);
  addresses = make_unique<char*[]>(argc);

  for (int i = 0; i < argc; i++) {
    argv[i] = _argv[i];
  }

  for (int i = argc - 1; i >= 0; i--) {
    size_t len = argv[i].size() + 1;
    len = round_up_to_multiple_of_n(len, 16);
    user_sp -= len;
    char* s = reinterpret_cast<char*>(user_sp);
    strcpy(s, argv[i].c_str());
    addresses[i] = s;
    printf("argv[%d] (0x%x) = %s\n", i, s, s);
  }

  user_sp -= round_up_to_multiple_of_n(sizeof(char*) * (argc + 2), 16);
  new_argv = reinterpret_cast<char**>(user_sp);
  for (int i = 0; i < argc; i++) {
    new_argv[i] = addresses[i];
  }
  new_argv[argc] = nullptr;

  new_argv_data = &new_argv[0];

out:
  user_sp -= 8;
  char*** new_argvp = reinterpret_cast<char***>(user_sp);
  *new_argvp = new_argv_data;

  user_sp -= 8;
  int* new_argc = reinterpret_cast<int*>(user_sp);
  *new_argc = argc;

  if (user_sp & 0xf) {
    Kernel::panic("sys_exec: user_sp misaligned!\n");
  }

  return user_sp;
}


void Task::handle_pending_signals() {
  Signal signal;

  while (!_pending_signals.empty()) {
    signal = _pending_signals.front();
    _pending_signals.pop_front();

    if (unlikely(!is_signal_valid(signal))) {
      Kernel::panic("invalid signal: 0x%x\n", signal);
    }

    if (_custom_signal_handlers[signal]) {
      _custom_signal_handlers[signal]();
    } else {
      invoke_default_signal_handler(signal);
    }
  }
}

}  // namespace valkyrie::kernel

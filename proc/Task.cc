// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

#include <String.h>
#include <fs/ELF.h>
#include <fs/Initramfs.h>
#include <kernel/ExceptionManager.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <libs/Math.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

// The pointer to the init process.
Task* Task::_init = nullptr;

// PID starts at 0 (idle task)
uint32_t Task::_next_pid = 0;


int Task::do_fork() {
  // Duplicate task
  printk("fork: saving parent cpu context\n");
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
    printk("do_fork: failed (out of memory).\n");
    return -1;
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
  child->_context.lr = reinterpret_cast<uint64_t>(&&child_return_to);
  child->_context.sp = child->_kstack_page.add_offset(kernel_sp_offset);

  // Calculate child's trap frame.
  // When the child returns from kernel mode to user mode,
  // child->_trap_frame->sp_el0 will be restored to the child
  // and used as the user mode SP.
  child->_trap_frame = child->_kstack_page.add_offset<TrapFrame*>(trap_frame_offset);
  child->_trap_frame->sp_el0 = child->_ustack_page.add_offset(user_sp_offset);

child_return_to:
  printf("pid: %d, ret = %d\n", Task::get_current()._pid, ret);
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
  printk("loading ELF at 0x%x\n", dest);

  if (!elf.is_valid()) {
    goto failed;
  }

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
  if (_active_children.empty()) {
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
  _state = Task::State::TERMINATED;
  _error_code = error_code;

  kfree(_elf_dest);

  auto& sched = TaskScheduler::get_instance();
  _parent->_active_children.remove(this);
  _parent->_terminated_children.push_back(sched.remove_task(*this));

  sched.schedule();
  Kernel::panic("sys_exit: returned from sched.\n");
}


size_t Task::construct_argv_chain(const char* const _argv[]) {
  // Construct the argv chain.
  size_t user_sp = _ustack_page.end();
  int argc = 0;
  char** addresses = nullptr;

  // The number of items to write on the stack.
  // i.e. argc itself, `argc` char pointers, and a nullptr sentinel.
  const size_t nr_items = argc + 2;


  if (_argv) {
    // Obtain `argc`.
    for (const char* s = _argv[0]; s; s = _argv[++argc]);

    // Copy all argv to kernel heap.
    String argv[argc];
    for (int i = 0; i < argc; i++) {
      argv[i] = _argv[i];
    }

    addresses = reinterpret_cast<decltype(addresses)>(
        kmalloc(sizeof(char*) * argc));

    for (int i = argc - 1; i >= 0; i--) {
      size_t len = argv[i].size() + 1;
      len = round_up_to_multiple_of_n(len, 16);
      user_sp -= len;
      char* s = reinterpret_cast<char*>(user_sp);
      strcpy(s, argv[i].c_str());
      addresses[i] = s;
      printf("argv[%d] = %s\n", i, s);
    }
  }


  // Calculate the final user SP value.
  user_sp -= nr_items * sizeof(size_t);

  // If `user_sp` is misaligned, minus 8 to make it aligned to 16 byte.
  if (user_sp & 0xf) {
    user_sp -= 8;
  }

  // Write `argc`.
  *reinterpret_cast<int*>(user_sp) = argc;

  if (_argv) {
    // Write `argv` pointers.
    for (int i = 0; i < argc; i++) {
      *reinterpret_cast<char**>(user_sp + 8 * (i + 1)) = addresses[i];
    }
  }

  // Write `argv` sentinel.
  *reinterpret_cast<char**>(user_sp + 8 * (argc + 1)) = nullptr;

  kfree(addresses);

  return user_sp;
}

}  // namespace valkyrie::kernel

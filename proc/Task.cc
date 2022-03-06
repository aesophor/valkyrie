// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

#include <Math.h>
#include <Mutex.h>
#include <String.h>

#include <fs/ELF.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <proc/TaskScheduler.h>

#define USER_BINARY_PAGE 0x400000
#define USER_STACK_PAGE 0x00007ffffffff000

namespace valkyrie::kernel {

// The pointer to the init and kthreadd task.
Task *Task::_init = nullptr;
Task *Task::_kthreadd = nullptr;

// PID starts at 0 (idle task)
uint32_t Task::_next_pid = 0;

Task::Task(bool is_user_task, Task *parent, void (*entry_point)(), const char *name)
    : _context(),
      _is_user_task(is_user_task),
      _parent(parent),
      _active_children(),
      _terminated_children(),
      _state(Task::State::CREATED),
      _error_code(),
      _pid(Task::_next_pid++),
      _time_slice(TASK_TIME_SLICE),
      _vmmap(),
      _entry_point(entry_point),
      _kstack_page(get_free_page(/*physical=*/true)),
      _ustack_page(get_free_page(/*physical=*/true)),
      _name(),
      _pending_signals(),
      _custom_signal_handlers(),
      _fd_table(),
      _cwd_vnode(VFS::the().get_rootfs().get_root_vnode()) {
  if (_pid == 1) [[unlikely]] {
    Task::_init = this;
  } else if (_pid == 2) [[unlikely]] {
    Task::_kthreadd = this;
  }

  if (parent) [[likely]] {
    parent->_active_children.push_back(this);
  }

  _context.lr = reinterpret_cast<size_t>(entry_point);
  _context.sp = _kstack_page.end();

  strcpy(_name, name);

  // Reserve fd 0,1,2 for stdin, stdout, stderr
  // FIXME: refactor this BULLSHIT
  auto opened = make_shared<File>(VFS::the().get_rootfs(), nullptr, 0);
  _fd_table[0] = opened;
  _fd_table[1] = opened;
  _fd_table[2] = opened;

#ifdef DEBUG
  printk(
      "constructed thread 0x%x [%s] (pid = %d): entry: 0x%x, _kstack_page = 0x%p, "
      "_ustack_page = 0x%p\n",
      this, _name, _pid, _entry_point, _kstack_page.begin(), _ustack_page.begin());
#endif
}

Task::~Task() {
#ifdef DEBUG
  printk("destructing thread 0x%x [%s] (pid = %d)\n", this, _name, _pid);
#endif

  // If the current task still has running children, make the init task adopt
  // these orphans and reap them.
  while (!_active_children.empty()) {
    auto child = _active_children.front();
    Task::_init->_active_children.push_back(child);
    child->_parent = Task::_init;
    _active_children.pop_front();
  }

  kfree(_kstack_page.p_addr());
}

List<Task *> Task::get_active_tasks() {
  List<Task *> ret;
  ret.push_back(Task::_init);
  ret.push_back(Task::_kthreadd);

  // Process tree BFS
  List<Task *> q;
  q.push_back(Task::_init);

  while (!q.empty()) {
    size_t size = q.size();

    for (size_t i = 0; i < size; i++) {
      Task *task = q.front();
      q.pop_front();

      ret.push_back(task);

      // Push all children onto the queue.
      for (const auto &child : task->_active_children) {
        q.push_back(child);
      }
    }
  }

  return ret;
}

Task *Task::get_by_pid(const pid_t pid) {
  if (pid == 1) {
    return Task::_init;
  } else if (pid == 2) {
    return Task::_kthreadd;
  }

  // Process tree BFS
  List<Task *> q;
  q.push_back(Task::_init);

  while (!q.empty()) {
    size_t size = q.size();

    for (size_t i = 0; i < size; i++) {
      Task *task = q.front();
      q.pop_front();

      auto it = task->_active_children.find_if([pid](auto t) { return t->_pid == pid; });

      if (it != task->_active_children.end()) {
        return *it;
      }

      // Push all children onto the queue.
      for (const auto &child : task->_active_children) {
        q.push_back(child);
      }
    }
  }

  return nullptr;
}

int Task::do_fork() {
#ifdef DEBUG
  printk("do_fork(): Forking from: %s (pid = %d)\n", get_name(), get_pid());
#endif

  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  save_context();

  size_t ret = 0;
  size_t parent_usp;
  asm volatile("mrs %0, sp_el0" : "=r"(parent_usp));

  size_t kernel_sp_offset = _kstack_page.offset_of(_context.sp);
  size_t trap_frame_offset = _kstack_page.offset_of(_trap_frame);

  // Clone task.
  auto task = make_unique<Task>(_is_user_task, /*parent=*/this, _entry_point, _name);
  Task *child = task.get();

  if (!task) {
    printk("do_fork: task object allocation failed (out of memory).\n");
    ret = -1;
    goto out;
  }

  if (!task->_kstack_page.p_addr()) {
    printk("do_fork: kernel stack allocation failed (out of memory).\n");
    ret = -1;
    goto out;
  }

  if (!task->_ustack_page.p_addr()) {
    printk("do_fork: user stack allocation failed (out of memory).\n");
    ret = -1;
    goto out;
  }

  // Enqueue the child task.
  TaskScheduler::the().enqueue_task(move(task));

  // Clone kernel/user stack content
  child->_kstack_page.copy_from(_kstack_page);
  child->_ustack_page.copy_from(_ustack_page);

  // ------ You can safely modify the child now ------

  // Set parent's fork() return value to child's pid.
  ret = child->_pid;

  // Clone child's CPU context.
  child->_context = _context;
  child->_context.lr = reinterpret_cast<uint64_t>(&&out);
  child->_context.sp = child->_kstack_page.add_offset(kernel_sp_offset);

  // Clone the page table using copy-on-write.
  child->_vmmap.copy_from(_vmmap);

  // Calculate child's trap frame.
  // When the child returns from kernel mode to user mode,
  // child->_trap_frame->sp_el0 will be restored to the child
  // and used as the user mode SP.
  child->_trap_frame = child->_kstack_page.add_offset<TrapFrame *>(trap_frame_offset);

  // Clone current working directory vnode.
  child->_cwd_vnode = _cwd_vnode;

  // TODO: Clone fd table

out:
  return ret;
}

int Task::do_exec(const char *name, const char *const _argv[]) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  // Update task name
  strncpy(_name, name, TASK_NAME_MAX_LEN - 1);

  // Acquire a new page and use it as the user stack page.
  _ustack_page = get_free_page(/*physical=*/true);

  // Construct the argv chain on the user stack.
  // Currently `_ustack_page` and `user_sp` contains physical addresses.
  size_t user_sp = copy_arguments_to_user_stack(_argv);
  size_t kernel_sp = KERNEL_VA_BASE + _kstack_page.end();

  // Convert `user_sp` to virtual addresses.
  size_t offset = _ustack_page.offset_of(user_sp);
  user_sp = USER_STACK_PAGE + offset;

  // Reset the stack pointer.
  _context.sp = user_sp;

  // Load the specified file from the filesystem.
  SharedPtr<File> file = VFS::the().open(name, 0);

  if (!file) {
    printk("exec failed: pid = %d [%s]. No such file or directory\n", _pid, _name);
    return -1;
  }

  // Map .text, .bss and .data
  ELF elf({file->vnode->get_content(), file->vnode->get_size()});

  if (!elf.is_valid()) {
    goto failed;
  }

  // Release the vmmap, freeing the old _ustack_page.
  _vmmap.reset();

  _ustack_page.set_v_addr(reinterpret_cast<void *>(USER_STACK_PAGE));
  _vmmap.map(USER_STACK_PAGE, _ustack_page.p_addr(), USER_PAGE_RWX);

  elf.load(_vmmap);

  VFS::the().close(move(file));

#ifdef DEBUG
  printk(
      "executing new program: %s <0x%x>, _kstack_page = 0x%x, _ustack_page = 0x%x, page_table "
      "= 0x%x\n",
      _name, elf.get_entry_point(), _kstack_page.begin(), _ustack_page.begin(),
      _vmmap.get_pgd());
#endif

  // Jump to the entry point.
  switch_to_user_mode(elf.get_entry_point(), user_sp, kernel_sp, _vmmap.get_pgd());

failed:
  printk("exec failed: pid = %d [%s]\n", _pid, _name);
  return -1;
}

int Task::do_wait(int *wstatus) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  if (!get_children_count()) {
    return -1;
  }

  // Suspends execution of the calling thread until one of its children terminates.
  while (_terminated_children.empty()) {
    TaskScheduler::the().schedule();
  }

  auto &child = _terminated_children.front();

  if (wstatus) {
    *wstatus = child->_error_code;
  }

  int ret = child->_pid;
  _terminated_children.pop_front();
  return ret;
}

[[noreturn]] void Task::do_exit(int error_code) {
  if (!_parent) [[unlikely]] {
    Kernel::panic("Attempted to kill %s! exit code=0x%08x\n", _name, error_code);
  }

  // Terminate the current task.
  _state = Task::State::TERMINATED;
  _error_code = error_code;

  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  // Close unclosed fds
  for (int i = 3; i < NR_TASK_FD_LIMITS; i++) {
    if (_fd_table[i]) {
      sys_close(i);
    }
  }

  auto &sched = TaskScheduler::the();
  _parent->_active_children.remove(this);
  _parent->_terminated_children.push_back(sched.remove_task(*this));

  sched.schedule();
  Kernel::panic("sys_exit: returned from sched.\n");
}

long Task::do_kill(pid_t pid, Signal signal) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  // Send a signal to `pid`.
  if (auto task = Task::get_by_pid(pid)) {
    task->_pending_signals.push_back(signal);
    return 0;
  }

  printk("sys_kill: failed (pid %d not found)\n", pid);
  return -1;
}

int Task::do_signal(int signal, void (*handler)()) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  if (!is_signal_valid(signal)) [[unlikely]] {
    printk("do_signal: failed (signal=0x%x is invalid)\n", signal);
    return -1;
  }

  _custom_signal_handlers[signal] = handler;
  return 0;  // TODO: return previous handler's error code.
}

size_t Task::copy_arguments_to_user_stack(const char *const argv[]) {
  int argc = 0;
  char **copied_argv = nullptr;
  char **copied_argv_ptr = nullptr;
  size_t user_sp = _ustack_page.end();
  UniquePtr<String[]> strings;
  UniquePtr<char *[]> copied_str_addrs;

  if (!argv) {
    goto out;
  }

  // Probe for argc from `_argv`.
  for (const char *s = argv[0]; s; s = argv[++argc])
    ;

  strings = make_unique<String[]>(argc);
  copied_str_addrs = make_unique<char *[]>(argc);

  // Copy all `argv` to kernel heap.
  for (int i = 0; i < argc; i++) {
    strings[i] = copy_from_user<const char *>(argv[i]);
  }

  // Actually copy all C strings to user stack, at the meanwhile save the new
  // addresses of C strings in `copied_str_addrs`.
  for (int i = argc - 1; i >= 0; i--) {
    size_t len = round_up_to_multiple_of_n(strings[i].size() + sizeof('\0'), 16);
    user_sp -= len;

    char *s = reinterpret_cast<char *>(user_sp);
    strcpy(reinterpret_cast<char *>(user_sp), strings[i].c_str());
    copied_str_addrs[i] = s;
  }

  // Subtract user SP and make it align to a 16-byte boundary.
  user_sp -= round_up_to_multiple_of_n(sizeof(char *) * (argc + 2), 16);

  // Now we will write the addrs in `copied_str_addrs` towards high address
  // starting from user SP.
  copied_argv = reinterpret_cast<char **>(user_sp);

  for (int i = 0; i < argc; i++) {
    copied_argv[i] = reinterpret_cast<char *>(
        USER_STACK_PAGE + (reinterpret_cast<size_t>(copied_str_addrs[i]) & 0xfff));
  }

  copied_argv[argc] = nullptr;

  copied_argv_ptr = reinterpret_cast<char **>(
      USER_STACK_PAGE + (reinterpret_cast<size_t>(&copied_argv[0]) & 0xfff));

out:
  user_sp -= 8;
  *reinterpret_cast<char ***>(user_sp) = copied_argv_ptr;

  user_sp -= 8;
  *reinterpret_cast<int *>(user_sp) = argc;

  if (user_sp & 0xf) {
    Kernel::panic("sys_exec: user SP misaligned!\n");
  }

  return user_sp;
}

void Task::handle_pending_signals() {
  Signal signal;

  while (!_pending_signals.empty()) {
    signal = _pending_signals.front();
    _pending_signals.pop_front();

    if (!is_signal_valid(signal)) [[unlikely]] {
      Kernel::panic("invalid signal: 0x%x\n", signal);
    }

    if (_custom_signal_handlers[signal]) {
      _custom_signal_handlers[signal]();
    } else {
      invoke_default_signal_handler(signal);
    }
  }
}

int Task::allocate_fd_for_file(SharedPtr<File> file) {
  for (int i = 0; i < NR_TASK_FD_LIMITS; i++) {
    if (!_fd_table[i]) {
      _fd_table[i] = move(file);

      if (!_fd_table[i]) {
        Kernel::panic("wtf\n");
      }
      return i;
    }
  }

  printk("warning: task (pid = %d) has reached fd limits!\n", _pid);
  return -1;
}

SharedPtr<File> Task::release_fd_and_get_file(const int fd) {
  return is_fd_valid(fd) ? move(_fd_table[fd]) : nullptr;
}

SharedPtr<File> Task::get_file_by_fd(const int fd) const {
  return is_fd_valid(fd) ? _fd_table[fd] : nullptr;
}

bool Task::is_fd_valid(const int fd) const {
  return fd >= 0 && fd < NR_TASK_FD_LIMITS;
}

}  // namespace valkyrie::kernel

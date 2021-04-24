// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

#include <String.h>
#include <fs/ELF.h>
#include <fs/Initramfs.h>
#include <kernel/ExceptionManager.h>
#include <libs/Math.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

// PID starts at 0 (idle task)
uint32_t Task::_next_pid = 0;


int Task::fork() {
  size_t ret = 0;

  size_t parent_usp;
  asm volatile("mrs %0, sp_el0" : "=r" (parent_usp));

  size_t user_sp_offset = _ustack_page.offset_of(parent_usp);
  size_t trap_frame_offset = _kstack_page.offset_of(_trap_frame);

  // Duplicate task
  printk("fork: saving parent cpu context\n");
  save_context();

  {
    printk("fork: start duplicating task\n");
    auto child = make_unique<Task>(this, _entry_point, _name);

    child->_trap_frame = child->_kstack_page.add_offset<TrapFrame*>(trap_frame_offset);
    printk("child trap frame = 0x%x\n", child->_trap_frame);

    // Copy kernel stack page content
    child->_kstack_page.copy_from(_kstack_page);

    // Copy user stack page content
    child->_ustack_page.copy_from(_ustack_page);

    // Set parent's fork() return value to child's pid.
    ret = child->_pid;

    // Calculate child's kernel SP.
    size_t child_ksp = child->_kstack_page.add_offset(
        _kstack_page.offset_of(_context.sp));

    // Copy child's CPU context.
    child->_context = _context;
    child->_context.lr = reinterpret_cast<uint64_t>(&&child_pc);
    child->_context.sp = child_ksp;

    // Enqueue the child task.
    TaskScheduler::get_instance().enqueue_task(move(child));
  }

child_pc:
  // The child will start executing from here.
  // However, the `this` pointer (which is stored in x0)
  // cannot be reassigned. Therefore, in order to
  // operate on the child, we have to explicitly operate
  // on the child's task stucture.
  if (ret == 0) {
    auto child = &Task::get_current();

    // Calculate child's user SP.
    size_t child_usp = child->_ustack_page.add_offset(user_sp_offset);
    asm volatile("msr SP_EL0, %0" :: "r" (child_usp));

    printk("parent usp: 0x%x\n", parent_usp);
    printk("_ustack_page = 0x%x, child usp: 0x%x\n", child->_ustack_page, child_usp);
    printk("setting child usp to 0x%x\n", child_usp);
  }

  printk("ret = %d\n", ret);
  return ret;
}


int Task::exec(const char* name, const char* const _argv[]) {
  size_t kernel_sp = _kstack_page.end();

  // Construct the argv chain on the user stack.
  size_t user_sp = construct_argv_chain(_argv);

  // Update task name
  strcpy(_name, name);

  // Reset the stack pointer.
  _context.sp = user_sp;

  // Load the specified file from the filesystem.
  ELF elf(Initramfs::get_instance().read(name));
  void* dest = reinterpret_cast<void*>(0x20000000);

  if (!elf.is_valid()) {
    goto failed;
  }

  elf.load_at(dest);

  // Jump to the entry point.
  printk("executing new program: %s, _kstack_page = 0x%x, _ustack_page = 0x%x\n",
         name, _kstack_page, _ustack_page);

  ExceptionManager::get_instance()
    .downgrade_exception_level(0,
                               elf.get_entry_point(dest),
                               reinterpret_cast<void*>(kernel_sp),
                               reinterpret_cast<void*>(user_sp));
failed:
  printk("exec failed: %s\n", name);
  return -1;
}

void Task::exit() {
  auto& sched = TaskScheduler::get_instance();

  sched.mark_terminated(*this);
  sched.schedule();
}


size_t Task::construct_argv_chain(const char* const _argv[]) {
  // Construct the argv chain.
  size_t user_sp = _ustack_page.end();

  int argc = 0;
  const char* s = _argv[0];

  while (s) {
    s = _argv[++argc];
  }

  String argv[argc];

  for (int i = 0; i < argc; i++) {
    argv[i] = _argv[i];
  }

  char* addresses[argc];

  for (int i = argc - 1; i >= 0; i--) {
    size_t len = argv[i].size() + 1;
    len = round_up_to_multiple_of_16(len);
    user_sp -= len;
    char* s = reinterpret_cast<char*>(user_sp);
    strcpy(s, argv[i].c_str());
    addresses[i] = s;
    printf("argv[i] = %s\n", s);
  }


  user_sp -= round_up_to_multiple_of_16(sizeof(char*) * (argc + 2));
  char** new_argv = reinterpret_cast<char**>(user_sp);
  for (int i = 0; i < argc; i++) {
    new_argv[i] = addresses[i];
  }
  new_argv[argc] = nullptr;

  user_sp -= 8;
  char*** new_argvp = reinterpret_cast<char***>(user_sp);
  *new_argvp = &new_argv[0];

  user_sp -= 8;
  int* new_argc = reinterpret_cast<int*>(user_sp);
  *new_argc = argc;

  return user_sp;
}

}  // namespace valkyrie::kernel

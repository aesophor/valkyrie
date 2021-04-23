// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

#include <String.h>
#include <dev/Console.h>
#include <fs/ELF.h>
#include <fs/Initramfs.h>
#include <kernel/ExceptionManager.h>
#include <libs/CString.h>
#include <libs/Math.h>
#include <mm/MemoryManager.h>
#include <proc/TaskScheduler.h>

extern "C" void switch_to(valkyrie::kernel::Task* prev,
                          valkyrie::kernel::Task* next);

namespace valkyrie::kernel {

// PID starts at 1
uint32_t Task::_next_pid = 1;


Task& Task::get_current() {
  Task* current;
  asm volatile("mrs %0, TPIDR_EL1" : "=r" (current));
  return *current;
}

void Task::set_current(const Task* t) {
  asm volatile("msr TPIDR_EL1, %0" :: "r" (t));
}


Task::Task(void* entry_point, const char* name)
    : _context(),
      _parent(),
      _state(Task::State::CREATED),
      _pid(Task::_next_pid++),
      _time_slice(3),
      _entry_point(reinterpret_cast<void*>(entry_point)),
      _kstack_page(get_free_page()),
      _ustack_page(get_free_page()),
      _name() {
  _context.lr = reinterpret_cast<uint64_t>(entry_point);
  _context.sp = reinterpret_cast<uint64_t>(_kstack_page) -
                PageFrameAllocator::get_block_header_size() +
                PAGE_SIZE;
  strcpy(_name, name);
  printk("constructed thread 0x%x [%s] (pid = %d): entry: 0x%x\n",
      this,
      _name,
      _pid,
      _entry_point);
}

Task::~Task() {
  printk("destructing thread 0x%x [%s] (pid = %d)\n",
      this,
      _name,
      _pid);

  kfree(_kstack_page);
  kfree(_ustack_page);
}


int Task::fork() {
  size_t ret = 0;

  // Duplicate task
  printk("saving parent cpu context\n");
  switch_to(&Task::get_current(), nullptr);  // save parent cpu context

  size_t sp_offset = _context.sp - reinterpret_cast<size_t>(_kstack_page);;

  {
    printk("duplicating child task\n");
    auto child = make_unique<Task>(_entry_point, _name);
    child->_parent = this;

    // Copy stack page content
    printk("memcpy(0x%x, 0x%x, 0x%x)\n", child->_kstack_page,
        _kstack_page,
        PAGE_SIZE - PageFrameAllocator::get_block_header_size());

    memcpy(child->_kstack_page,
           _kstack_page,
           PAGE_SIZE - PageFrameAllocator::get_block_header_size());

    memcpy(child->_ustack_page,
           _ustack_page,
           PAGE_SIZE - PageFrameAllocator::get_block_header_size());

    // Set parent's fork() return value to child's pid.
    ret = child->_pid;

    // Copy child's CPU context.
    child->_context.x19 = _context.x19;
    child->_context.x20 = _context.x20;
    child->_context.x21 = _context.x21;
    child->_context.x22 = _context.x22;
    child->_context.x23 = _context.x23;
    child->_context.x24 = _context.x24;
    child->_context.x25 = _context.x25;
    child->_context.x26 = _context.x26;
    child->_context.x27 = _context.x27;
    child->_context.x28 = _context.x28;
    child->_context.fp = _context.fp;
    child->_context.lr = reinterpret_cast<uint64_t>(&&child_pc);
    child->_context.sp = reinterpret_cast<uint64_t>(child->_kstack_page) + sp_offset;

    // Enqueue the child task.
    printk("enque task...\n");
    TaskScheduler::get_instance().enqueue_task(move(child));
  }

  // The child will start executing from here.
child_pc:
  printk("child ready to return!\n");
  return ret;
}

int Task::exec(const char* name, const char* const _argv[]) {
  size_t kernel_sp = reinterpret_cast<size_t>(_kstack_page) -
                     PageFrameAllocator::get_block_header_size() +
                     PAGE_SIZE;

  // Construct the argv chain.
  size_t user_sp = reinterpret_cast<size_t>(_ustack_page) -
                   PageFrameAllocator::get_block_header_size() +
                   PAGE_SIZE;

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

  // Reset the stack pointer.
  _context.sp = user_sp;

  // Load the specified file from the filesystem.
  ELF elf(Initramfs::get_instance().read(name));
  size_t dest_addr = 0x20000000;
  void* dest = reinterpret_cast<void*>(dest_addr);

  if (!elf.is_valid()) {
    goto failed;
  }
  elf.load_at(dest);

  // Jump to the entry point.
  printk("executing new program: %s, _ustack_page = 0x%x, sp = 0x%x\n",
      name, _ustack_page, _context.sp);
 
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
  _state = Task::State::TERMINATED;
}


void Task::construct_argv_chain(const char* const _argv[]) {
}

}  // namespace valkyrie::kernel

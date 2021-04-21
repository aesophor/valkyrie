// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

#include <String.h>
#include <dev/Console.h>
#include <kernel/Kernel.h>
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
      _state(Task::State::CREATED),
      _pid(Task::_next_pid++),
      _time_slice(3),
      _entry_point(reinterpret_cast<void*>(entry_point)),
      _stack_page(get_free_page()),
      _name() {
  _context.lr = reinterpret_cast<uint64_t>(entry_point);
  _context.sp = reinterpret_cast<uint64_t>(_stack_page) -
                PageFrameAllocator::get_block_header_size() +
                PAGE_SIZE;
  strcpy(_name, name);
  printk("constructed thread 0x%x [%s] (pid = %d)\n",
      this,
      _name,
      _pid);
}

Task::~Task() {
  printk("destructing thread 0x%x [%s] (pid = %d)\n",
      this,
      _name,
      _pid);

  kfree(_stack_page);
}


int Task::fork() const {
  size_t ret = 0;

  // Duplicate task
  printk("saving parent cpu context\n");
  switch_to(&Task::get_current(), nullptr);  // save parent cpu context

  size_t sp_offset = _context.sp - reinterpret_cast<size_t>(_stack_page);;

  {
    printk("duplicating child task\n");
    auto child = make_unique<Task>(_entry_point, _name);

    // Copy stack page content
    printk("memcpy(0x%x, 0x%x, 0x%x)\n", child->_stack_page,
        _stack_page,
        PAGE_SIZE - PageFrameAllocator::get_block_header_size());

    memcpy(child->_stack_page,
        _stack_page,
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
    child->_context.sp = reinterpret_cast<uint64_t>(child->_stack_page) + sp_offset;

    // Enqueue the child task.
    printk("enque task...\n");
    TaskScheduler::get_instance().enqueue_task(move(child));
  }

  // The child will start executing from here.
child_pc:
  printk("child ready to return!\n");
  return ret;
}

int Task::exec(void (*func)(), const char* const _argv[]) {
  // Construct the argv chain.
  size_t sp = reinterpret_cast<size_t>(_stack_page) -
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
    sp -= len;
    char* s = reinterpret_cast<char*>(sp);
    strcpy(s, argv[i].c_str());
    addresses[i] = s;
    printf("argv[i] = %s\n", s);
  }


  sp -= round_up_to_multiple_of_16(sizeof(char*) * (argc + 2));
  char** new_argv = reinterpret_cast<char**>(sp);
  //new_argv[0] = new_argv[1];
  for (int i = 0; i < argc; i++) {
    new_argv[i] = addresses[i];
  }
  new_argv[argc] = nullptr;

  sp -= 16;
  int* new_argc = reinterpret_cast<int*>(sp);
  *new_argc = argc;

  // Reset the stack pointer.
  _context.sp = sp;

  // Jump to the entry point.
  printf("trying to exec the new program...\n");
  asm volatile("mov x0, %0\n\
                mov x1, %1\n\
                mov lr, %2\n\
                blr lr" :: "r" (argc), "r" (new_argv), "r" (func));

  return -1;
}

void Task::exit() {
  _state = Task::State::TERMINATED;
}

}  // namespace valkyrie::kernel

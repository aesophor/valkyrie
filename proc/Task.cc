// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

#include <dev/Console.h>
#include <libs/CString.h>
#include <libs/Math.h>
#include <mm/MemoryManager.h>

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


Task::Task(void *entry_point, const char* name)
    : _context(),
      _state(Task::State::CREATED),
      _pid(Task::_next_pid++),
      _time_slice(3),
      _entry_point(entry_point),
      _stack_page(get_free_page()),
      _name() {
  _context.lr = reinterpret_cast<uint64_t>(entry_point);
  _context.sp = reinterpret_cast<uint64_t>(_stack_page) -
                PageFrameAllocator::get_block_header_size() +
                PAGE_SIZE;
  strcpy(_name, name);
}

Task::~Task() {
  kfree(_stack_page);
}


int Task::exec(void (*func)(), const char* const argv[]) {
  // Construct the argv chain.
  size_t sp = reinterpret_cast<size_t>(_stack_page) -
              PageFrameAllocator::get_block_header_size() +
              PAGE_SIZE;

  int argc = 0;
  const char* s = argv[0];
  char* addresses[argc];

  while (s) {
    s = argv[++argc];
  }

  for (int i = argc - 1; i >= 0; i--) {
    size_t len = strlen(argv[i]) + 1;
    len = round_up_to_multiple_of_16(len);
    sp -= len;
    char* s = reinterpret_cast<char*>(sp);
    strcpy(s, argv[i]);
    addresses[i] = s;
  }


  sp -= round_up_to_multiple_of_16(sizeof(char*) * (argc + 2));
  char** new_argv = reinterpret_cast<char**>(sp);
  new_argv[0] = new_argv[1];
  new_argv[1] = addresses[0];
  new_argv[2] = addresses[1];
  new_argv[3] = nullptr;

  sp -= 16;
  int* new_argc = reinterpret_cast<int*>(sp);
  *new_argc = argc;

  // Reset the stack pointer.
  _context.sp = sp;

  // Jump to the entry point.
  printf("trying to exec the new program...\n");
  func();

  return -1;
}

void Task::exit() {
  _state = Task::State::TERMINATED;
}

}  // namespace valkyrie::kernel

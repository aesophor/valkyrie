// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

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


Task::Task(void *entry_point)
    : _context(),
      _state(Task::State::CREATED),
      _pid(Task::_next_pid++),
      _time_slice(3),
      _stack_page(get_free_page()) {
  _context.lr = reinterpret_cast<uint64_t>(entry_point);
  _context.sp = reinterpret_cast<uint64_t>(_stack_page) -
                PageFrameAllocator::get_block_header_size() +
                PAGE_SIZE;
}

Task::~Task() {
  kfree(_stack_page);
}


uint32_t Task::get_pid() const {
  return _pid;
}

}  // namespace valkyrie::kernel

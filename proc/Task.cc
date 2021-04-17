// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Task.h>

namespace valkyrie::kernel {

// PID starts at 1
uint32_t Task::_next_pid = 1;


Task* Task::get_current() {
  Task* current;
  asm volatile("mrs %0, TPIDR_EL1" : "=r"(current));
  return current;
}

void Task::set_current(const Task* t) {
  asm volatile("msr TPIDR_EL1, %0" :: "r"(t));
}


void* Task::get_task_n_stack_top(const int pid) {
  return reinterpret_cast<void*>(TASK_0_STACK_TOP + TASK_STACK_SIZE * pid);
}

void* Task::get_task_n_stack_bottom(const int pid) {
  return reinterpret_cast<void*>(TASK_0_STACK_TOP + (TASK_STACK_SIZE) * (pid + 1));
}



Task::Task(void (*entry_point)())
    : _state(Task::State::CREATED),
      _pid(Task::_next_pid++),
      _entry_point(entry_point),
      _stack_top(get_task_n_stack_top(_pid)),
      _stack_bottom(get_task_n_stack_bottom(_pid)),
      _stack_pointer(_stack_bottom) {}


uint32_t Task::get_pid() const {
  return _pid;
}

}  // namespace valkyrie::kernel

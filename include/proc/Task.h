// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <Types.h>
#include <mm/PageFrameAllocator.h>

#define TASK_0_STACK_TOP HEAP_END
#define TASK_STACK_SIZE  8192

namespace valkyrie::kernel {

class Task {
 public:
  enum class State {
    CREATED,
    RUNNABLE,
    WAITING,
    ZOMBIE,
    SIZE
  };

  // Constructor
  Task(void (*entry_point)());

  // Destructor
  ~Task() = default;

  static Task* get_current();
  static void  set_current(const Task* t);

  static void* get_task_n_stack_bottom(const int pid);
  static void* get_task_n_stack_top(const int pid);

  uint32_t get_pid() const;

 private:
  static uint32_t _next_pid;

  Task::State _state;
  uint32_t _pid;
  void (*_entry_point)();
  void* _stack_top;
  void* _stack_bottom;
  void* _stack_pointer;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_

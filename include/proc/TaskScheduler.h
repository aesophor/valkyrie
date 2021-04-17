// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_SCHEDULER_H_
#define VALKYRIE_TASK_SCHEDULER_H_

#include <Deque.h>
#include <proc/Task.h>

namespace valkyrie::kernel {

class TaskScheduler {
 public:
  static TaskScheduler& get_instance();
  ~TaskScheduler() = default;

  // Starts the task scheduler.
  void run();

  void enqueue_task(UniquePtr<Task> task);
  void switch_to(Task* prev, Task* next);

 private:
  TaskScheduler();

  Deque<UniquePtr<Task>> _run_queue;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_SCHEDULER_H_

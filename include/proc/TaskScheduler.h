// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_SCHEDULER_H_
#define VALKYRIE_TASK_SCHEDULER_H_

#include <List.h>
#include <Memory.h>
#include <Mutex.h>
#include <proc/Task.h>

namespace valkyrie::kernel {

class TaskScheduler {
 public:
  static TaskScheduler& get_instance();

  ~TaskScheduler() = default;
  TaskScheduler(const TaskScheduler&) = delete;
  TaskScheduler(TaskScheduler&&) = delete;
  TaskScheduler& operator =(const TaskScheduler&) = delete;
  TaskScheduler& operator =(TaskScheduler&&) = delete;

  // Starts the task scheduler.
  void run();

  void enqueue_task(UniquePtr<Task> task);
  UniquePtr<Task> remove_task(const Task& task);

  void schedule();
  void maybe_reschedule();
  void tick();

 private:
  TaskScheduler();

  bool _need_reschedule;
  List<UniquePtr<Task>> _runqueue;
  Mutex _m;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_SCHEDULER_H_

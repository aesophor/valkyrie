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

  // Starts the task scheduler.
  void run();

  /*
  template <typename F>
  Task& create_kernel_thread(F func, const char* name) {
    return enqueue_task(make_unique<Task>(&Task::get_kthreadd(), func, name));
  }
  */

  Task& enqueue_task(UniquePtr<Task> task);
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

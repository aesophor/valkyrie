// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_SCHEDULER_H_
#define VALKYRIE_TASK_SCHEDULER_H_

#include <List.h>
#include <Memory.h>
#include <Mutex.h>
#include <Singleton.h>

#include <proc/Task.h>

namespace valkyrie::kernel {

class TaskScheduler : public Singleton<TaskScheduler> {
 public:
  // Starts the task scheduler.
  void run();

  void enqueue_task(UniquePtr<Task> task);
  UniquePtr<Task> remove_task(const Task &task);

  void schedule();
  void maybe_reschedule();
  void tick();

 protected:
  TaskScheduler();

 private:
  bool _need_reschedule;
  List<UniquePtr<Task>> _runqueue;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_SCHEDULER_H_

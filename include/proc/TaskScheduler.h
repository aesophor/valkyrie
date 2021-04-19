// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_SCHEDULER_H_
#define VALKYRIE_TASK_SCHEDULER_H_

#include <DoublyLinkedList.h>
#include <proc/Task.h>

namespace valkyrie::kernel {

class TaskScheduler {
 public:
  static TaskScheduler& get_instance();
  ~TaskScheduler() = default;

  // Starts the task scheduler.
  void run();

  void enqueue_task(UniquePtr<Task> task);
  void remove_task(Task* task);
  void mark_as_zombie(Task& task);
  void schedule();
  void reap_zombies();

 private:
  TaskScheduler();

  DoublyLinkedList<UniquePtr<Task>> _run_queue;
  DoublyLinkedList<Task*> _zombies;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_SCHEDULER_H_

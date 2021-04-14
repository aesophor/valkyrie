// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASKLET_SCHEDULER_H_
#define VALKYRIE_TASKLET_SCHEDULER_H_

#include <Memory.h>
#include <RingBuffer.h>
#include <dev/Console.h>
#include <kernel/Tasklet.h>

namespace valkyrie::kernel {

class TaskletScheduler {
 public:
  TaskletScheduler();
  ~TaskletScheduler() = default;

  template <typename T>
  void schedule(T&& handler) {
    if (_tasklet_queue.full()) {
      printk("tasklet queue is full. calling do_all()...\n");
      finish_all();
    }

    printk("adding a tasklet to the queue...\n");
    _tasklet_queue.push(make_unique<Tasklet>(forward<T>(handler)));
  }

  void finish_all();

 private:
  RingBuffer<UniquePtr<Tasklet>> _tasklet_queue;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASKLET_SCHEDULER_H_

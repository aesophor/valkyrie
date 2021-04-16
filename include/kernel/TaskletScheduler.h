// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// A tasklet is a special form of deferred work that runs in interrupt context,
// just like softirqs. The main difference between sofirqs and tasklets is that
// tasklets can be allocated dynamically and thus they can be used by device drivers.
//
// Reference:
// [1] https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab4.html
// [2] https://linux-kernel-labs.github.io/refs/heads/master/labs/deferred_work.html#tasklets

#ifndef VALKYRIE_TASKLET_SCHEDULER_H_
#define VALKYRIE_TASKLET_SCHEDULER_H_

#include <Memory.h>
#include <RingBuffer.h>
#include <dev/Console.h>
#include <kernel/Tasklet.h>

namespace valkyrie::kernel {

class TaskletScheduler {
 public:
  TaskletScheduler() : _tasklet_queue() {}
  ~TaskletScheduler() = default;


  template <typename T>
  void add_tasklet(T&& handler) {
    if (_tasklet_queue.full()) {
      printk("tasklet queue is full. calling do_all()...\n");
      finish_all();
    }

    printk("adding a tasklet to the queue...\n");
    _tasklet_queue.push(make_unique<Tasklet>(forward<T>(handler)));
  }


  void finish_all() {
    for (size_t i = 0; i < _tasklet_queue.size(); i++) {
      printk("handling tasklet %d ... ", i);
      _tasklet_queue[i]->handle();
      _tasklet_queue[i].reset();
    }
    _tasklet_queue.clear();
  }

 private:
  RingBuffer<UniquePtr<Tasklet>> _tasklet_queue;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASKLET_SCHEDULER_H_

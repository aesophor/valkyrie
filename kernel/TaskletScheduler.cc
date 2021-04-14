// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// A tasklet is a special form of deferred work that runs in interrupt context,
// just like softirqs. The main difference between sofirqs and tasklets is that
// tasklets can be allocated dynamically and thus they can be used by device drivers.
//
// Reference:
// [1] https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab4.html
// [2] https://linux-kernel-labs.github.io/refs/heads/master/labs/deferred_work.html#tasklets

#include <kernel/TaskletScheduler.h>

#include <dev/Console.h>

namespace valkyrie::kernel {

TaskletScheduler::TaskletScheduler() : _tasklet_queue() {}


void TaskletScheduler::schedule(UniquePtr<Tasklet> tasklet) {
  if (_tasklet_queue.full()) {
    printk("tasklet queue is full. calling do_all()...\n");
    do_all();
  }

  _tasklet_queue.push(move(tasklet));
  printk("added a tasklet to the queue\n");
}

void TaskletScheduler::do_all() {
  for (size_t i = 0; i < _tasklet_queue.size(); i++) {
    printk("handling tasklet %d ...", i);
    _tasklet_queue[i]->handle();
    _tasklet_queue[i].reset();
  }
  _tasklet_queue.clear();
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/TaskletScheduler.h>

#include <kernel/Kernel.h>

namespace valkyrie::kernel {

TaskletScheduler::TaskletScheduler() : _tasklet_queue() {}


void TaskletScheduler::schedule(Tasklet&& tasklet) {
  _tasklet_queue.push_back(forward<Tasklet>(tasklet));
  printk("added a tasklet to the queue.\n");
}

void TaskletScheduler::do_all() {
  for (size_t i = 0; i < _tasklet_queue.size(); i++) {
    printk("handling tasklet %d\n", i);
    _tasklet_queue[i].handle();
  }
  _tasklet_queue.clear();
}

}  // namespace valkyrie::kernel

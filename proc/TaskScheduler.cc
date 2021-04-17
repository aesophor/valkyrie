// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/TaskScheduler.h>

#include <dev/Console.h>
#include <usr/Shell.h>

namespace valkyrie::kernel {

TaskScheduler& TaskScheduler::get_instance() {
  static TaskScheduler instance;
  return instance;
}

TaskScheduler::TaskScheduler()
    : _run_queue() {}


void TaskScheduler::run() {
  printk("scheduler: started\n");

  printk("scheduler: added shell task\n");
  enqueue_task(make_unique<Task>(run_shell));

  for (;;) {

  }
}


void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  _run_queue.push_back(move(task));
}

}  // namespace valkyrie::kernel

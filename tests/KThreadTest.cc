// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/Console.h>
#include <dev/IO.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

void foo() {
  for (int i = 0; i < 10; ++i) {
    printf("Thread id: %d %d\n", Task::get_current().get_pid(), i);
    io::delay(1000000);
    TaskScheduler::get_instance().schedule();
  }
  Task::get_current().do_exit(0);
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/idle.h>

#include <kernel/Kernel.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

[[noreturn]] void idle() {
  auto& exmgr = ExceptionManager::get_instance();
  auto& sched = TaskScheduler::get_instance();

  while (true) {
    sched.reap_zombies();
//    printf("~~~~~~~~~~~~~~~~~~~~~~~~~ preemption ON ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    exmgr.enable();
    sched.schedule();
  }
}

}  // namespace valkyrie::kernel

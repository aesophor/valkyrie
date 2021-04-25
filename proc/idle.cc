// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/idle.h>

#include <kernel/Kernel.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

[[noreturn]] void idle() {
  auto& exmgr = ExceptionManager::get_instance();
  auto& sched = TaskScheduler::get_instance();

  while (true) {
    /*
    size_t spsr;
    asm volatile("mrs %0, spsr_el1" : "=r" (spsr));
    printf("spsr = 0x%x\n", spsr);
    */

    exmgr.enable();
    sched.reap_zombies();
    sched.schedule();
  }
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/idle.h>

#include <kernel/ExceptionManager.h>
#include <kernel/Kernel.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

[[noreturn]] void idle() {
  while (true) {
    ExceptionManager::enableIRQs();
    TaskScheduler::the().schedule();
  }
}

}  // namespace valkyrie::kernel

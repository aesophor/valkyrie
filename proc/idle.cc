// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/idle.h>

#include <kernel/Kernel.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

[[noreturn]] void idle() {
  while (true) {
    ExceptionManager::enable();
    TaskScheduler::get_instance().schedule();
  }
}

}  // namespace valkyrie::kernel

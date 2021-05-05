// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/start_kthreadd.h>

#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

[[noreturn]] void start_kthreadd() {
  while (true) {
    // Wait for any kthread to terminate.
    Task::current()->do_wait(nullptr);
    TaskScheduler::get_instance().schedule();
  }
}

}  // namespace valkyrie::kernel

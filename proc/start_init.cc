// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/start_init.h>

#include <kernel/Kernel.h>
#include <kernel/Syscall.h>

#define INIT_PATH "/sbin/init"

namespace valkyrie::kernel {

// Must be started as a kthread.
// See TaskScheduler::run().

[[noreturn]] void start_init() {
  const char* argv[] = {INIT_PATH, "param1", "test", nullptr};
  Task::current()->do_exec(INIT_PATH, argv);

  // sys_exec() shouldn't have returned.
  Kernel::panic("no working init found.\n");
}

}  // namespace valkyrie::kernel

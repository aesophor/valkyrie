// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Syscall.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

void sys_irq() {
  printk("ARM core timer enabled.\n");
  ExceptionManager::get_instance()->get_arm_core_timer().enable();
}

}  // namespace valkyrie::kernel

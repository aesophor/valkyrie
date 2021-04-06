// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Syscall.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

void sys_timer_irq_enable() {
  printk("ARM core timer enabled.\n");
  ExceptionManager::get_instance()->get_arm_core_timer().enable();
}

void sys_timer_irq_disable() {
  printk("ARM core timer disabled.\n");
  ExceptionManager::get_instance()->get_arm_core_timer().disable();
}

}  // namespace valkyrie::kernel

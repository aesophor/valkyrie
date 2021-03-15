// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Syscall.h>

#include <Console.h>
#include <Kernel.h>

namespace valkyrie::kernel {

void sys_irq() {
  printk("ARM core timer enabled.\n");
  Kernel::get_instance()->get_exception_manager()->get_arm_core_timer().enable();
}

}  // namespace valkyrie::kernel

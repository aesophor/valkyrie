// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Syscall.h>

#include <Console.h>
#include <Kernel.h>

namespace valkyrie::kernel {

void syscall(const size_t x0,
             const size_t x1,
             const size_t x2,
             const size_t x3,
             const size_t x4,
             const size_t x5,
             const size_t id) {
  /*
  // Trigger the syscall.
  // This will trigger an exception, causing the CPU
  // to switch from userspace to kernel space, and
  // according to `id` the corresponding syscall will be invoke.
  asm volatile("svc #0");
  */

  switch (id) {
    case 0:
      sys_irq();
      break;

    default:
      printk("unknown syscall id: 0x%d\n", id);
      break;
  }
}

void sys_irq() {
  printk("ARM core timer enabled.");
  Kernel::get_instance()->get_exception_manager()->get_arm_core_timer().enable();
}

}  // namespace valkyrie::kernel

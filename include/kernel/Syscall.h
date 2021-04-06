// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SYSCALL_H_
#define VALKYRIE_SYSCALL_H_

#include <Types.h>
#include <dev/Console.h>

namespace valkyrie::kernel {

// Individual system call declaration.
void sys_timer_irq_enable();
void sys_timer_irq_disable();


// Indirect system call
//
// A system call is issued using the `svc 0` instruction.
// The system call number is passed via register x1,
// the parameters are stored in x2 ~ x7,
// and the return value will be stored in x0.
template <typename... Args>
void syscall(const size_t number, const Args ...args) {
  switch (number) {
    case 0:
      sys_timer_irq_enable();
      break;

    case 1:
      sys_timer_irq_disable();
      break;

    default:
      printk("bad system call: (id=0x%x)\n", number);
      break;
  }
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SYSCALL_H_

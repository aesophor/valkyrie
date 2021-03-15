// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SYSCALL_H_
#define VALKYRIE_SYSCALL_H_

#include <Console.h>
#include <Types.h>

namespace valkyrie::kernel {

// Individual system call declaration.
void sys_irq();


// Indirect system call
//
// A system call is issued using the `svc 0` instruction.
// The system call number is passed via register x1,
// the parameters are stored in x2 ~ x7,
// and the return value will be stored in x0.
template <typename... Args>
uint64_t syscall(const size_t number, const Args ...args) {
  switch (number) {
    case 0:
      sys_irq();
      break;

    default:
      printk("bad system call: (id=0x%x)\n", number);
      break;
  }
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SYSCALL_H_

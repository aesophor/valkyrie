// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SYSCALL_H_
#define VALKYRIE_SYSCALL_H_

#include <Types.h>

namespace valkyrie::kernel {

// Valkyrie's syscall convention is the same as that of Linux.
//
// A system call is issued using the `svc 0` instruction.
// The system call number is passed on register x8,
// the parameters are stored in x0 ~ x5,
// and the return value will be stored in x0.
void syscall(const size_t x0,
             const size_t x1,
             const size_t x2,
             const size_t x3,
             const size_t x4,
             const size_t x5,
             const size_t id);

void sys_irq();

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SYSCALL_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// When a user process takes an exception and enters kernel mode,
// the registers are saved at the kernel stack’s top.
// Before returning to the user mode, the registers are loaded.
// The saved content is called the trap frame.
//
// In regular exception handling(e.g. page fault, interrupt),
// the kernel won’t touch the trap frame, so the user process
// won’t notice that it entered the kernel mode.
// However, in the case of system calls, the user program expects that
// the kernel does something for it.
//
// As regular function calls, the program sets the arguments and
// gets the return value by accessing the general-purpose registers.
// Then, the kernel can read the trap frame to get the user’s arguments and
// write the trap frame to set the return value and the error code.

#ifndef VALKYRIE_TRAP_FRAME_H_
#define VALKYRIE_TRAP_FRAME_H_

#include <Types.h>

namespace valkyrie::kernel {

struct TrapFrame final {
  uint64_t x0;
  uint64_t x1;
  uint64_t x2;
  uint64_t x3;
  uint64_t x4;
  uint64_t x5;
  uint64_t x6;
  uint64_t x7;
  uint64_t x8;
  uint64_t x9;
  uint64_t x10;
  uint64_t x11;
  uint64_t x12;
  uint64_t x13;
  uint64_t x14;
  uint64_t x15;
  uint64_t x16;
  uint64_t x17;
  uint64_t x18;
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t x29;
  uint64_t spsr_el1;
  uint64_t elr_el1;
  uint64_t sp_el0;
  uint64_t x30;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TRAP_FRAME_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// boot.S - Valkyrie Kernel Entry Point
//
// The GPU of rpi3 will place the kernel8.img at 0x80000
// and start executing the code there. In our case,
// 0x80000 is the address of `_start` (see scripts/linker.ld for details).
//
// We will set the SP (Stack Pointer) to 0x80000 and let it grow towards 0,
// initialize the .bss segment to zeroes by calling memset,
// and finally branch to the `kmain()` function.

.extern memset

.section ".text"
_start:
  // Initialize the stack pointer to the address of _start,
  // allowing it to grow toward lower address.
  ldr x0, = _start
  mov sp, x0

  // Initialize .bss by calling memset()
  ldr x0, = _bss_start
  mov x1, 0
  ldr x2, = _bss_end
  sub x2, x2, x0
  bl memset

  // Transfer control to kmain()
  bl kmain

_halt:
  wfe
  b _halt

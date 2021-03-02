// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.

.section ".text"

.extern memset

_start:
  // Initialize the stack pointer to the address of __kernel_loader,
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

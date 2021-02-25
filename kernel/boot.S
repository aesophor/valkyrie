// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.

.section ".text"

.extern _ZN8valkyrie6kernel6memsetEPvhy

.global __kernel_loader
__kernel_loader:
  // Initialize the stack pointer to the address of __kernel_loader,
  // allowing it to grow toward lower address.
  ldr x0, = __kernel_loader
  mov sp, x0

__initialize_bss:
1:
  ldr x0, = _bss_start
  mov x1, 0
  ldr x2, = _bss_end
  sub x2, x2, x0
  bl _ZN8valkyrie6kernel6memsetEPvhy

  bl kmain

__kernel_end:
  wfe
  b __kernel_end

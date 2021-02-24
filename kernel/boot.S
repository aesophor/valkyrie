// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.

.section ".text"

.global __kernel_loader
__kernel_loader:
  // Initialize the stack pointer to the address of __kernel_loader,
  // allowing it to grow toward lower address.
  ldr x0, = __kernel_loader
  mov sp, x0

1:
  // kmain(_bss_start, _bss_end)
  ldr w0, = _bss_start
  ldr w1, = _bss_end
  bl kmain

__kernel_end:
  wfe
  b __kernel_end

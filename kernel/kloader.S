// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.

.section ".text"

_start:
  // Initialize stack pointer
  ldr x0, = _stack_top
  mov sp, x0

1:
  // kmain(_bss_start, _bss_end)
  ldr w0, = _bss_start
  ldr w1, = _bss_end
  bl kmain

end_kernel:
  wfe
  b end_kernel

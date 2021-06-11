// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// va_space.S - switch virtual address space

.section ".text"
.global switch_to_user_mode
switch_to_user_mode:
  msr ELR_EL1, x0
  msr SP_EL0, x1
  mov x4, #0
  msr SPSR_EL1, x4
  mov SP, x2

  dsb ish
  msr TTBR0_EL1, x3
  tlbi vmalle1is
  dsb ish
  isb

  eret

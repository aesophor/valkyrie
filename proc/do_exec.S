// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// do_exec.S - switch to user mode and continue execution there.

.section ".text"
.global switch_to_user_mode
switch_to_user_mode:
  msr elr_el1, x0
  msr sp_el0, x1
  mov x4, #0
  msr spsr_el1, x4
  mov SP, x2

  dsb ish
  msr ttbr0_el1, x3
  tlbi vmalle1is
  dsb ish
  isb

  eret

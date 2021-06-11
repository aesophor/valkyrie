// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// va_space.S - switch virtual address space

.section ".text"
.global switch_user_va_space
switch_user_va_space:
  dsb ish            // ensure write has completed
  msr TTBR0_EL1, x0  // switch translation based address.
  tlbi vmalle1is     // invalidate all TLB entries
  dsb ish            // ensure completion of TLB invalidatation
  isb                // clear pipeline

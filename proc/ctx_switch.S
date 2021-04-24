// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// ctx_switch.S - Context Switch

.section ".text"
.global switch_to
switch_to:
  cmp x0, #0
  beq __restore
  
  cmp x0, x1
  beq __restore

__save:
  stp x19, x20, [x0, 16 * 0]
  stp x21, x22, [x0, 16 * 1]
  stp x23, x24, [x0, 16 * 2]
  stp x25, x26, [x0, 16 * 3]
  stp x27, x28, [x0, 16 * 4]
  stp fp, lr, [x0, 16 * 5]
  mov x9, sp
  str x9, [x0, 16 * 6]

__restore:
  cmp x1, #0
  beq __out

  ldp x19, x20, [x1, 16 * 0]
  ldp x21, x22, [x1, 16 * 1]
  ldp x23, x24, [x1, 16 * 2]
  ldp x25, x26, [x1, 16 * 3]
  ldp x27, x28, [x1, 16 * 4]
  ldp fp, lr, [x1, 16 * 5]
  ldr x9, [x1, 16 * 6]
  mov sp,  x9
  msr TPIDR_EL1, x1

__out:
  ret

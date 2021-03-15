// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// esr.S - Exception Service Routine

.macro save_cpu_context
  sub	sp, sp, 256
  stp	x0, x1, [sp, 16 * 0]
  stp	x2, x3, [sp, 16 * 1]
  stp	x4, x5, [sp, 16 * 2]
  stp	x6, x7, [sp, 16 * 3]
  stp	x8, x9, [sp, 16 * 4]
  stp	x10, x11, [sp, 16 * 5]
  stp	x12, x13, [sp, 16 * 6]
  stp	x14, x15, [sp, 16 * 7]
  stp	x16, x17, [sp, 16 * 8]
  stp	x18, x19, [sp, 16 * 9]
  stp	x20, x21, [sp, 16 * 10]
  stp	x22, x23, [sp, 16 * 11]
  stp	x24, x25, [sp, 16 * 12]
  stp	x26, x27, [sp, 16 * 13]
  stp	x28, x29, [sp, 16 * 14]
  str	x30, [sp, 16 * 15] 
.endm

.macro restore_cpu_context
  ldp	x0, x1, [sp, 16 * 0]
  ldp	x2, x3, [sp, 16 * 1]
  ldp	x4, x5, [sp, 16 * 2]
  ldp	x6, x7, [sp, 16 * 3]
  ldp	x8, x9, [sp, 16 * 4]
  ldp	x10, x11, [sp, 16 * 5]
  ldp	x12, x13, [sp, 16 * 6]
  ldp	x14, x15, [sp, 16 * 7]
  ldp	x16, x17, [sp, 16 * 8]
  ldp	x18, x19, [sp, 16 * 9]
  ldp	x20, x21, [sp, 16 * 10]
  ldp	x22, x23, [sp, 16 * 11]
  ldp	x24, x25, [sp, 16 * 12]
  ldp	x26, x27, [sp, 16 * 13]
  ldp	x28, x29, [sp, 16 * 14]
  ldr	x30, [sp, 16 * 15] 
  add	sp, sp, 256
.endm


.section ".text"
.global handle_exception
handle_exception:
  save_cpu_context
  // `x0` should be `this` pointer
  bl _ZN8valkyrie6kernel16ExceptionManager12get_instanceEv
  bl _ZN8valkyrie6kernel16ExceptionManager16handle_exceptionEyyyyyyy
  restore_cpu_context
  eret

.global handle_irq
handle_irq:
  save_cpu_context
  // `x0` should be `this` pointer
  bl _ZN8valkyrie6kernel16ExceptionManager12get_instanceEv
  bl _ZN8valkyrie6kernel16ExceptionManager10handle_irqEv
  restore_cpu_context
  eret

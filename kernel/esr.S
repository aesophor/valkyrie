// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// esr.S - Exception Service Routine

#define REGISTER_SAVE_LOAD_BYTES 272

.macro save_cpu_context
  sub	sp, sp, REGISTER_SAVE_LOAD_BYTES
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
  // Begin saving SPSR_EL1 and ELR_EL1
  mrs   x28, SPSR_EL1
  mrs   x29, ELR_EL1
  stp   x28, x29, [sp, 16 * 15]
  // End saving SPSR_EL1 and ELR_EL1
  mrs   x29, SP_EL0
  stp   x29, x30, [sp, 16 * 16]
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
  // Begin restoring SPSR_EL1 and ELR_EL1
  ldp   x28, x29, [sp, 16 * 15]
  msr   SPSR_EL1, x28
  msr   ELR_EL1, x29
  // End restoring SPSR_EL1 and ELR_EL1
  ldp   x29, x30, [sp, 16 * 16]
  msr   SP_EL0, x29
  ldp	x28, x29, [sp, 16 * 14]
  add	sp, sp, REGISTER_SAVE_LOAD_BYTES
.endm

.macro set_x0_to_trapframe_addr
  mov x0, sp
.endm


.section ".text"
.global handle_exception
handle_exception:
  save_cpu_context
  set_x0_to_trapframe_addr
  bl _ZN8valkyrie6kernel16ExceptionManager16handle_exceptionEPNS0_9TrapFrameE
  restore_cpu_context
  eret

.global handle_irq
handle_irq:
  save_cpu_context
  bl _ZN8valkyrie6kernel16ExceptionManager10handle_irqEv
  restore_cpu_context
  eret

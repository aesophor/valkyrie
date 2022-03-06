// Copyright (c) 2021-2022 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// boot.S - Valkyrie Kernel Entry Point
//
// See scripts/linker.ld for details.

.section ".text"
.global _start
_start:
  // Let core with cpuid != 0 enter busy loop
  mrs x0, mpidr_el1
  and x0, x0, 3
  cbnz x0, _ZN8valkyrie6kernel6Kernel4haltEv

  // Allow access to variadic functions in EL1.
  // On Arm64, when we want to print out some message, the va_list will
  // use the SIMD&FP registers (like q0, q1) to store parameters. So, we
  // have to disable the trap of accessing floating-point and Advanced SIMD
  // registers to make va_list use SIMD&FP registers properly.
  // See: https://lists.xen.org/archives/html/minios-devel/2018-07/msg00038.html
  mrs x0, cpacr_el1
  orr x0, x0, #(0b11 << 20)
  msr cpacr_el1, x0

  // Configure HCR_EL2 (Hypervisor Configuration Register - EL2)
  // by setting HCR_EL2.RW to 1 (which means EL1 is AArch64)
  mrs x0, hcr_el2
  orr x0, x0, #(1 << 31)
  msr hcr_el2, x0

  // Install exception vector table.
  ldr x0, = evt
  msr vbar_el1, x0

  // Drop exception level to EL1 and enable MMU. See boot/mmu.S
  mov x0, 0
  orr x0, x0, #(1 << 0)
  orr x0, x0, #(1 << 2)
  orr x0, x0, #(0b1111 << 6)
  msr spsr_el2, x0
  adr x0, __mmu_init
  msr elr_el2, x0
  adr x0, _start
  msr sp_el1, x0
  eret

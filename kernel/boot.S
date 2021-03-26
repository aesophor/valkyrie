// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// boot.S - Valkyrie Kernel Entry Point
//
// The GPU of rpi3 will place the kernel8.img at 0x80000
// and start executing the code there. In our case,
// 0x80000 is the address of `_start` (see scripts/linker.ld for details).
//
// We will do the following tasks in boot.S:
// 1. Set the SP (Stack Pointer) to 0x80000 and let it grow towards 0
// 2. initialize the .bss segment to zeroes by calling memset
// 3. configure CPACR_EL1 to allow the use of variadic functions
// 4. configure HCR_EL2.RW to 1 to run at EL1 in 64-bit mode
// 5. finally, branch to the `kmain()` function.

.extern memset

.section ".text"
.global _start
_start:
  // Let core with cpuid != 0 enter busy loop
  mrs x0, mpidr_el1
  and x0, x0, 3
  cbnz x0, _halt

  // Initialize the stack pointer to the address of _start,
  // allowing it to grow toward lower address.
  ldr x0, = _start
  mov sp, x0

  // Initialize .bss by calling memset()
  ldr x0, = _bss_start
  mov x1, #0
  ldr x2, = _bss_end
  sub x2, x2, x0
  bl memset

  // Allow access to variadic functions in EL1.
  // On Arm64, when we want to print out some message, the va_list will
  // use the SIMD&FP registers (like q0, q1) to store parameters. So, we
  // have to disable the trap of accessing floating-point and Advanced SIMD
  // registers to make va_list use SIMD&FP registers properly.
  // See: https://lists.xen.org/archives/html/minios-devel/2018-07/msg00038.html
  mrs x0, CPACR_EL1
  orr x0, x0, #(0b11 << 20)
  msr CPACR_EL1, x0

  // Configure HCR_EL2 (Hypervisor Configuration Register - EL2)
  // by setting HCR_EL2.RW to 1 (which means EL1 is AArch64)
  mrs x0, HCR_EL2
  orr x0, x0, #(1 << 31)
  msr HCR_EL2, x0

  // Transfer control to kmain()
  bl kmain

.global _halt
_halt:
  wfe
  b _halt

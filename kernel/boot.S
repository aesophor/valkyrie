// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// boot.S - Valkyrie Kernel Entry Point
//
// The GPU of rpi3 will place the kernel8.img at 0x80000
// and start executing the code there. In our case,
// 0x80000 is the address of `_start` (see scripts/linker.ld for details).
//
// We will set the SP (Stack Pointer) to 0x80000 and let it grow towards 0,
// initialize the .bss segment to zeroes by calling memset,
// and finally branch to the `kmain()` function.

.extern memset
.extern evt

.section ".text"
_start:
  // Initialize the stack pointer to the address of _start,
  // allowing it to grow toward lower address.
  ldr x0, = _start
  mov sp, x0

  // Initialize .bss by calling memset()
  ldr x0, = _bss_start
  mov x1, 0
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
  orr x0, x0, (0b11 << 20)
  msr CPACR_EL1, x0

  // Configure HCR_EL2 (Hypervisor Configuration Register - EL2)
  // by setting HCR_EL2.RW to 1 (which means EL1 is AArch64)
  mrs x0, HCR_EL2
  orr x0, x0, (1 << 31)
  msr HCR_EL2, x0

  // Setup EL1 stack
  ldr x0, = _start
  msr SP_EL1, x0

  // Install the exception vector table to VBAR_EL1
  ldr x0, = evt
  msr VBAR_EL1, x0

  // Configure SPSR_EL2 (Saved Program Status Register - EL2)
  // See ARMv8 manual p.387 for details.
  mrs x0, SPSR_EL2
  orr x0, x0, (1 << 0)  // SP_ELx
  orr x0, x0, (1 << 2)  // EL1
  orr x0, x0, (0b1111 << 6)
  msr SPSR_EL2, x0

  // Configure ELR_EL2 which holds the return address of `eret`
  ldr x0, = __el1_entry
  msr ELR_EL2, x0

  // Switch from EL2 to EL1
  eret

__el1_entry:
  mov x0, 0x40000
  msr sp_el0, x0
  mov x0, 0 // EL0 with interrupt enabled
  msr spsr_el1, x0
  ldr x0, = __el0_entry
  msr elr_el1, x0
  eret

__el0_entry:
  // Transfer control to kmain()
  bl kmain

.global _halt
_halt:
  wfe
  b _halt

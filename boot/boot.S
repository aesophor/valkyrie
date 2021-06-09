// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// boot.S - Valkyrie Kernel Entry Point
//
// The GPU of rpi3 will place the kernel8.img at 0x80000
// and start executing the code there.
//
// In our case, the physical address of `_start` is 0x80000,
// whereas the virtual address of `_start` is 0xffff000000080000.
// All symbols in build/valkyrie.eif only exists at 0xffff*
// because it is compiled in that way (see scripts/linker.ld for details).
//
// We will do the following tasks in boot.S:
// 1. switch the Exception Level from EL2 (hypervisor) to EL1 (supervisor).
// 2. configure CPACR_EL1 to allow the use of variadic functions
// 3. configure HCR_EL2.RW to 1 to run at EL1 in 64-bit mode
// 4. branch to `__mmu_init()`, which initializes and turns on MMU.
// 5. set the SP (Stack Pointer) to 0xffff000000080000.
// 6. initialize the .bss segment to zeroes by calling memset
// 7. transfer control to `kmain()`.

.section ".text"
.global _start
_start:
  // Let core with cpuid != 0 enter busy loop
  mrs x0, mpidr_el1
  and x0, x0, 3
  cbnz x0, _halt

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

__switch_to_EL1:
  ldr x0, = evt
  msr VBAR_EL1, x0

  mov x0, 0
  orr x0, x0, #(1 << 0)
  orr x0, x0, #(1 << 2)
  orr x0, x0, #(0b1111 << 6)
  msr spsr_el2, x0

  adr x0, __mmu_init
  msr elr_el2, x0

  ldr x0, = _start - 0x10  // sets SP to 0xffff000000080000 - 0x10
  msr sp_el1, x0
  eret  // eret to `__mmu_init()`, which will branch back to `__boot_rest()`

.global __boot_rest
__boot_rest:
  // Transfer control to kmain(), see kernel/kmain.cc
  bl kmain

.global _halt
_halt:
  wfe
  b _halt

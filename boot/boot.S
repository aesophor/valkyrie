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

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_KERNEL_USER (1 << 6)
#define PD_RDONLY (1 << 7)
#define PD_ACCESS (1 << 10)

#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)

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


setup_EL1:
  ldr x0, = evt
  msr VBAR_EL1, x0

  mov x0, 0 // EL1h (SPSel = 1) with interrupt disabled
  orr x0, x0, #(1 << 0)
  orr x0, x0, #(1 << 2)
  orr x0, x0, #(0b1111 << 6)
  msr spsr_el2, x0

  adr x0, __mmu_init
  msr elr_el2, x0

  # setup EL1 stack
  ldr x0, = _start - 0x1000
  msr sp_el1, x0
  eret


__mmu_init:
  ldr x0, = TCR_CONFIG_DEFAULT
  msr tcr_el1, x0

  ldr x0, =( \
      (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
      (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
  )
  msr mair_el1, x0


  mov x0, 0x0000 // PGD's page frame at 0x0
  mov x1, 0x1000 // PUD's page frame at 0x1000

  ldr x2, = BOOT_PGD_ATTR
  orr x2, x1, x2 // combine the physical address of next level page with attribute.
  str x2, [x0]

  ldr x2, = BOOT_PUD_ATTR
  mov x3, 0x00000000
  orr x3, x2, x3
  str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD

  mov x3, 0x40000000
  orr x3, x2, x3
  str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

  msr ttbr0_el1, x0 // load PGD to the bottom translation based register.

  mrs x2, sctlr_el1
  orr x2 , x2, 1
  msr sctlr_el1, x2 // enable MMU, cache remains disabled

  // Indirect branch to the virtual address of `__boot_rest`,
  // now we have successfully enabled virtual memory.
  ldr x2, = __boot_rest
  blr x2

__boot_rest:
  // Initialize the stack pointer to the virtual address of _start,
  // allowing it to grow toward lower address.
  ldr x0, = _start
  mov sp, x0

  // Initialize .bss by calling memset()
  ldr x0, = _bss_start
  mov x1, #0
  ldr x2, = _bss_end
  sub x2, x2, x0
  bl memset

  // Transfer control to kmain(), see kernel/kmain.cc
  bl kmain


.global _halt
_halt:
  wfe
  b _halt

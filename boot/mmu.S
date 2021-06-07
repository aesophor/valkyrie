// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// mmu.S - Set up Kernel Space Virtual Memory

// MMU Configuration
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB          ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT      (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

// Memory Attribute Indirection Register (MAIR)
#define MAIR_DEVICE_nGnRnE      0b00000000
#define MAIR_NORMAL_NOCACHE     0b01000100
#define MAIR_IDX_DEVICE_nGnRnE  0
#define MAIR_IDX_NORMAL_NOCACHE 1

// Page Directory Attributes
#define PD_TABLE       0b11
#define PD_PAGE        0b11
#define PD_BLOCK       0b01
#define PD_KERNEL_USER (1 << 6)
#define PD_RDONLY      (1 << 7)
#define PD_ACCESS      (1 << 10)

// The addresses of Kernel PGD, PUD, PMD
#define KERNEL_PGD_PTR 0x0000
#define KERNEL_PUD_PTR 0x1000
#define KERNEL_PMD_PTR 0x2000

#define BOOT_PGD_ATTR PD_TABLE
#define DEVICE_MEMORY_ATTR ((MAIR_IDX_DEVICE_nGnRnE << 2) | PD_ACCESS | PD_BLOCK)
#define NORMAL_MEMORY_ATTR ((MAIR_IDX_NORMAL_NOCACHE << 2) | PD_ACCESS | PD_BLOCK)


.section ".text"
.global __mmu_init
__mmu_init:
  ldr x0, = TCR_CONFIG_DEFAULT
  msr tcr_el1, x0

  ldr x0, =( \
      (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
      (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
  )
  msr mair_el1, x0

  // Create kernel page tables and load its address to
  // the translation base register of the upper va_space (i.e. kernel space).
  bl __create_kernel_page_tables
  msr ttbr1_el1, x0

  // Enable MMU with cache disabled.
  mrs x2, sctlr_el1
  orr x2 , x2, 1
  msr sctlr_el1, x2

  // Indirect branch to the virtual address of `__boot_rest`.
  // This symbol is defined in boot/boot.S
  ldr x2, = __boot_rest
  br x2


__create_kernel_page_tables:
  // Write a PGD at 0x0000
  mov x0, KERNEL_PGD_PTR
  mov x1, KERNEL_PUD_PTR
  ldr x2, = PD_TABLE
  orr x2, x1, x2
  str x2, [x0]

  // Write PUD at 0x1000
  mov x0, KERNEL_PUD_PTR
  mov x1, KERNEL_PMD_PTR
  ldr x2, = PD_TABLE
  orr x2, x1, x2
  str x2, [x0]

  // Write PMD at 0x2000
  mov x0, KERNEL_PMD_PTR
  mov x1, 0
  ldr x2, = NORMAL_MEMORY_ATTR
  orr x2, x1, x2
  str x2, [x0]

  // ARM local peripherals (0x40000000 - 0x7fffffff)
  mov x0, KERNEL_PUD_PTR + 8
  mov x1, 0x40000000
  ldr x2, = DEVICE_MEMORY_ATTR
  orr x2, x1, x2
  str x2, [x0]

  // GPU peripherals (0x3f000000 - 0x3fffffff)
  mov x0, KERNEL_PMD_PTR + (0x3f000000 / 0x200000) * 8
  mov x1, 0x3f000000
  ldr x2, = DEVICE_MEMORY_ATTR
  orr x2, x1, x2
  str x2, [x0]

  mov x0, KERNEL_PMD_PTR + (0x3f200000 / 0x200000) * 8
  mov x1, 0x3f200000
  ldr x2, = DEVICE_MEMORY_ATTR
  orr x2, x1, x2
  str x2, [x0]

  // Zone1
  mov x0, KERNEL_PMD_PTR + (0x10000000 / 0x200000) * 8
  mov x1, 0x10000000
  ldr x2, = NORMAL_MEMORY_ATTR
  orr x2, x1, x2
  str x2, [x0]

  // Zone2
  mov x0, KERNEL_PMD_PTR + (0x10200000 / 0x200000) * 8
  mov x1, 0x10200000
  ldr x2, = NORMAL_MEMORY_ATTR
  orr x2, x1, x2
  str x2, [x0]


  // Return the address of PGD
  mov x0, KERNEL_PGD_PTR
  ret

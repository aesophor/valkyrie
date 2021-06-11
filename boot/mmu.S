// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/mmu.h>

// MMU Configuration
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB          ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT      (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

// The addresses of Kernel PGD, PUD, PMD
#define KERNEL_PGD_PTR 0x0000
#define KERNEL_PUD_PTR 0x1000
#define KERNEL_PMD_PTR 0x2000

#define DEVICE_MEMORY_ATTR ((MAIR_IDX_DEVICE_nGnRnE << 2) | PD_ACCESS | PD_BLOCK)
#define NORMAL_MEMORY_ATTR ((MAIR_IDX_NORMAL_NOCACHE << 2) | PD_ACCESS | PD_BLOCK)

.macro WRITE_PAGE_DESCRIPTOR write_to_addr physical_addr attributes
  mov x0, \write_to_addr
  mov x1, \physical_addr
  ldr x2, = \attributes
  orr x2, x1, x2
  str x2, [x0]
.endm


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
  msr ttbr0_el1, x0
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
  WRITE_PAGE_DESCRIPTOR KERNEL_PGD_PTR, KERNEL_PUD_PTR, PD_TABLE

  // Write PUD at 0x1000
  WRITE_PAGE_DESCRIPTOR KERNEL_PUD_PTR, KERNEL_PMD_PTR, PD_TABLE

  // Write PMD at 0x2000
  WRITE_PAGE_DESCRIPTOR KERNEL_PMD_PTR, 0, NORMAL_MEMORY_ATTR

  // ARM local peripherals (0x40000000 - 0x7fffffff)
  WRITE_PAGE_DESCRIPTOR (KERNEL_PUD_PTR + 8), 0x40000000, DEVICE_MEMORY_ATTR

  // GPU peripherals (0x3f000000 - 0x3fffffff)
  WRITE_PAGE_DESCRIPTOR (KERNEL_PMD_PTR + (0x3f000000 / 0x200000) * 8), 0x3f000000, DEVICE_MEMORY_ATTR
  WRITE_PAGE_DESCRIPTOR (KERNEL_PMD_PTR + (0x3f200000 / 0x200000) * 8), 0x3f200000, DEVICE_MEMORY_ATTR

  // Kernel heap
  WRITE_PAGE_DESCRIPTOR (KERNEL_PMD_PTR + (0x10000000 / 0x200000) * 8), 0x10000000, NORMAL_MEMORY_ATTR
  WRITE_PAGE_DESCRIPTOR (KERNEL_PMD_PTR + (0x10200000 / 0x200000) * 8), 0x10200000, NORMAL_MEMORY_ATTR

  // Return the address of PGD
  mov x0, KERNEL_PGD_PTR
  ret

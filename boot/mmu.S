// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// mmu.S - Set up Kernel Space Virtual Memory

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

  // Enable MMU; cache remains disabled.
  mrs x2, sctlr_el1
  orr x2 , x2, 1
  msr sctlr_el1, x2

  // Indirect branch to the virtual address of `__boot_rest`.
  // This symbol is defined in boot/boot.S
  ldr x2, = __boot_rest
  blr x2


__create_kernel_page_tables:
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
  ret

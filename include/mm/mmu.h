// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MMU_H_
#define VALKYRIE_MMU_H_

// Memory Attribute Indirection Register (MAIR)
#define MAIR_DEVICE_nGnRnE      0b00000000
#define MAIR_NORMAL_NOCACHE     0b01000100
#define MAIR_IDX_DEVICE_nGnRnE  0
#define MAIR_IDX_NORMAL_NOCACHE 1

// Page Directory Attributes
#define PD_INVALID(x)  (((x) & 1) == 0)
#define PD_BLOCK       0b01
#define PD_TABLE       0b11
#define PD_PAGE        0b11
#define PD_KERNEL_USER (1 << 6)
#define PD_RDONLY      (1 << 7)
#define PD_ACCESS      (1 << 10)

// Helper macros for extracting indices/offset from a virtual address.
#define PGD_INDEX(x) (((x) >> 39) & 0x1ff)  // PGD index
#define PUD_INDEX(x) (((x) >> 30) & 0x1ff)  // PUD index
#define PMD_INDEX(x) (((x) >> 21) & 0x1ff)  // PMD index
#define PTE_INDEX(x) (((x) >> 12) & 0x1ff)  // PTE index
#define PAGE_OFFSET(x) ((x) & 0xfff)        // offset within page

// Page Table Entry Mask
#define PAGE_MASK 0x00007ffffffff000  // [47:12] physical address
#define ATTR_MASK 0x0000000000000fff  // [11: 2] attributes

#endif  // VALKYRIE_MMU_H_

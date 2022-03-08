// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// https://developer.arm.com/documentation/102376/0100/Describing-memory-in-AArch64
#ifndef VALKYRIE_MMU_H_
#define VALKYRIE_MMU_H_

#define KERNEL_VA_BASE 0xffff000000000000

// Memory Attribute Indirection Register (MAIR)
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

// Page descriptor's attributes
#define PD_COW_PAGE (1UL << 55)
#define PD_EL0_EXEC_NEVER (1UL << 54)
#define PD_EL1_EXEC_NEVER (1UL << 53)
#define PD_ACCESS (1UL << 10)
#define PD_RDONLY (1UL << 7)
#define PD_KERNEL_USER (1UL << 6)
#define PD_INVALID(x) (((x) &1) == 0)
#define PD_BLOCK 0b01
#define PD_TABLE 0b11
#define PD_PAGE 0b11

// Page permissions
#define __USER_PAGE ((MAIR_IDX_NORMAL_NOCACHE << 2) | PD_ACCESS | PD_KERNEL_USER | PD_PAGE)
#define USER_PAGE_RWX (__USER_PAGE)
#define USER_PAGE_RX (__USER_PAGE | PD_RDONLY)
#define USER_PAGE_RW (__USER_PAGE | PD_EL0_EXEC_NEVER)
#define USER_PAGE_R (__USER_PAGE | PD_ONLY | PD_EL0_EXEC_NEVER)

// Helper macros for extracting indices/offset from a virtual address.
#define PGD_INDEX(x) (((x) >> 39) & 0x1ff)  // PGD index
#define PUD_INDEX(x) (((x) >> 30) & 0x1ff)  // PUD index
#define PMD_INDEX(x) (((x) >> 21) & 0x1ff)  // PMD index
#define PTE_INDEX(x) (((x) >> 12) & 0x1ff)  // PTE index
#define PAGE_OFFSET(x) ((x) &0xfff)         // offset within page

// Page size and Page Table Entry (PTE) masks.
#define PAGE_SHIFT 12
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE - 1))

// When we are manipulating a page descriptor, the bits [58:55] are reserved for
// software use, so we shouldn't use the regular PAGE_MASK. Instead, we define a
// special "physical" page mask which extracts the physical address from the bits
// [47:12] of a page descriptor.
#define PD_PAGE_MASK 0x00007ffffffff000
#define PD_ATTR_MASK (PAGE_SIZE - 1)

#endif  // VALKYRIE_MMU_H_

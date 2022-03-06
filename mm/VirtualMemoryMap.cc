// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/VirtualMemoryMap.h>

#include <CString.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <mm/MemoryManager.h>

#define PAGE_TABLE_DEPTH 4
#define NR_ENTRIES_PER_PT (PAGE_SIZE / sizeof(size_t))

namespace valkyrie::kernel {

VMMap::VMMap() : _pgd(reinterpret_cast<pagetable_t *>(get_free_page(true))) {
  if (!_pgd) [[unlikely]] {
    printk("error: unable to allocate pgd\n");
    return;
  }

  memset(_pgd, 0, PAGE_SIZE);
}

VMMap::~VMMap() {
  reset();
  kfree(_pgd);
}

void VMMap::reset() const {
  dfs_kfree_page(_pgd, 0);
  memset(_pgd, 0, PAGE_SIZE);
}

void VMMap::map(const size_t v_addr, const void *p_addr, const size_t attr) const {
#ifdef DEBUG
  printk("[%s] VMMap::map: v_addr = 0x%p, p_addr = 0x%p\n", Task::current()->get_name(), v_addr, p_addr);
#endif

  // Extract the page table indices from `v_addr`.
  size_t pt_indices[PAGE_TABLE_DEPTH] = {
      PGD_INDEX(v_addr),
      PUD_INDEX(v_addr),
      PMD_INDEX(v_addr),
      PTE_INDEX(v_addr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  pagetable_t *pt = _pgd;     // current page table
  size_t pt_index;            // page table index
  size_t pd;                  // page descriptor
  size_t next_level_pt_addr;  // addr of next level page table

  for (int i = 0; i < PAGE_TABLE_DEPTH - 1; i++) {
    pt_index = pt_indices[i];
    pd = pt[pt_index];

    // If this page descriptor is invalid, then it indicates that the next-level
    // page table is not present yet. Hence, we should allocate the next-level
    // page table now.
    if (PD_INVALID(pd)) {
      void *page_frame = get_free_page(true);
      memset(page_frame, 0, PAGE_SIZE);
      next_level_pt_addr = reinterpret_cast<size_t>(page_frame);
      pt[pt_index] = next_level_pt_addr | PD_TABLE;
    } else {
      next_level_pt_addr = pt[pt_index] & PAGE_MASK;
    }

    // Advance to the next level page table.
    pt = reinterpret_cast<pagetable_t *>(next_level_pt_addr);
  }

  // We've reached the last-level page table.
  pt_index = pt_indices[PAGE_TABLE_DEPTH - 1];
  pd = pt[pt_index];

  if (!PD_INVALID(pd)) [[unlikely]] {
    Kernel::panic("VMMap::map: v_addr: 0x%p has already been mapped to p_addr: 0x%p\n", v_addr,
                  p_addr);
    return;
  }

  MemoryManager::the().inc_page_ref_count(p_addr);

  pt[pt_index] = reinterpret_cast<size_t>(p_addr) | attr | PD_COW_PAGE;
}

void VMMap::unmap(const size_t v_addr) const {
#ifdef DEBUG
  printk("[%s] VMMap::unmap: v_addr = 0x%p\n", Task::current()->get_name(), v_addr);
#endif

  // Extract the page table indices from `v_addr`.
  size_t pt_indices[PAGE_TABLE_DEPTH] = {
      PGD_INDEX(v_addr),
      PUD_INDEX(v_addr),
      PMD_INDEX(v_addr),
      PTE_INDEX(v_addr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  pagetable_t *pt = _pgd;     // current page table
  size_t pt_index;            // page table index
  size_t pd;                  // page descriptor
  size_t next_level_pt_addr;  // addr of next level page table

  for (int i = 0; i < PAGE_TABLE_DEPTH - 1; i++) {
    pt_index = pt_indices[i];
    pd = pt[pt_index];

    // If this page descriptor is invalid, then it indicates that the next-level
    // page table is not present yet. Hence, we can return immediately.
    if (PD_INVALID(pd)) {
      Kernel::panic("VMMap::map: v_addr: 0x%p has not been mapped yet...\n", v_addr);
      return;
    }

    // Advance to the next level page table.
    next_level_pt_addr = pt[pt_index] & PAGE_MASK;
    pt = reinterpret_cast<pagetable_t *>(next_level_pt_addr);
  }

  // We've reached the last-level page table.
  pt_index = pt_indices[PAGE_TABLE_DEPTH - 1];
  pd = pt[pt_index];

  if (PD_INVALID(pd)) [[unlikely]] {
    printk("VMMap::map: v_addr: 0x%p has not been mapped yet...\n", v_addr);
    return;
  }

  size_t page_frame_addr = reinterpret_cast<size_t>(pt[pt_index] & PAGE_MASK);
  void *p_addr = reinterpret_cast<void *>(page_frame_addr);
  MemoryManager::the().dec_page_ref_count(p_addr);

  pt[pt_index] = 0;
}

bool VMMap::is_cow_page(const void *const __v_addr) const {
  const size_t v_addr = reinterpret_cast<const size_t>(__v_addr);

  // Extract the page table indices from `v_addr`.
  size_t pt_indices[PAGE_TABLE_DEPTH] = {
      PGD_INDEX(v_addr),
      PUD_INDEX(v_addr),
      PMD_INDEX(v_addr),
      PTE_INDEX(v_addr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  pagetable_t *pt = _pgd;     // current page table
  size_t pt_index;            // page table index
  size_t pd;                  // page descriptor
  size_t next_level_pt_addr;  // addr of next level page table

  for (int i = 0; i < PAGE_TABLE_DEPTH - 1; i++) {
    pt_index = pt_indices[i];
    pd = pt[pt_index];

    // If this page descriptor is invalid, then it indicates that the next-level page table
    // is not present yet. Hence, we can return immediately.
    if (PD_INVALID(pd)) {
      printk("VMMap: v_addr: 0x%x has not been mapped yet...\n", v_addr);
      return reinterpret_cast<void *>(v_addr);
    }

    // Advance to the next level page table.
    next_level_pt_addr = pt[pt_index] & PAGE_MASK;
    pt = reinterpret_cast<pagetable_t *>(next_level_pt_addr);
  }

  // We've reached the last-level page table.
  pt_index = pt_indices[PAGE_TABLE_DEPTH - 1];
  pd = pt[pt_index];

  if (PD_INVALID(pd)) [[unlikely]] {
    printk("VMMap: v_addr: 0x%p has not been mapped yet...\n", v_addr);
    return reinterpret_cast<void *>(v_addr);
  }

  return pd & PD_COW_PAGE;
}

void *VMMap::get_physical_address(const void *const __v_addr) const {
  const size_t v_addr = reinterpret_cast<const size_t>(__v_addr);

  // Extract the page table indices from `v_addr`.
  size_t pt_indices[PAGE_TABLE_DEPTH] = {
      PGD_INDEX(v_addr),
      PUD_INDEX(v_addr),
      PMD_INDEX(v_addr),
      PTE_INDEX(v_addr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  pagetable_t *pt = _pgd;     // current page table
  size_t pt_index;            // page table index
  size_t pd;                  // page descriptor
  size_t next_level_pt_addr;  // addr of next level page table

  for (int i = 0; i < PAGE_TABLE_DEPTH - 1; i++) {
    pt_index = pt_indices[i];
    pd = pt[pt_index];

    // If this page descriptor is invalid, then it indicates that the next-level page table
    // is not present yet. Hence, we can return immediately.
    if (PD_INVALID(pd)) {
      // XXX: VMMap: v_addr: 0xb0058 has not been mapped yet...
      //printk("VMMap: v_addr: 0x%x has not been mapped yet...\n", v_addr);
      return reinterpret_cast<void *>(v_addr);
    }

    // Advance to the next level page table.
    next_level_pt_addr = pt[pt_index] & PAGE_MASK;
    pt = reinterpret_cast<pagetable_t *>(next_level_pt_addr);
  }

  // We've reached the last-level page table.
  pt_index = pt_indices[PAGE_TABLE_DEPTH - 1];
  pd = pt[pt_index];

  if (PD_INVALID(pd)) [[unlikely]] {
    printk("VMMap: v_addr: 0x%p has not been mapped yet...\n", v_addr);
    return reinterpret_cast<void *>(v_addr);
  }

  size_t p_addr = (pt[pt_index] & PAGE_MASK) + PAGE_OFFSET(reinterpret_cast<size_t>(v_addr));

  return reinterpret_cast<void *>(p_addr);
}

void VMMap::copy_from(const VMMap &r) const {
  // 1. Copy the page frames of page tables.
  // 2. Mark PTEs of both child & parent to read-only even for original read-write pages.
  dfs_copy_page_tables(r._pgd, _pgd, 0);
}

void VMMap::copy_page_frame(const void *const __v_addr) const {
  const size_t v_addr = reinterpret_cast<const size_t>(__v_addr);

  // Extract the page table indices from `v_addr`.
  size_t pt_indices[PAGE_TABLE_DEPTH] = {
      PGD_INDEX(v_addr),
      PUD_INDEX(v_addr),
      PMD_INDEX(v_addr),
      PTE_INDEX(v_addr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  pagetable_t *pt = _pgd;     // current page table
  size_t pt_index;            // page table index
  size_t pd;                  // page descriptor
  size_t next_level_pt_addr;  // addr of next level page table

  for (int i = 0; i < PAGE_TABLE_DEPTH - 1; i++) {
    pt_index = pt_indices[i];
    pd = pt[pt_index];

    // If this page descriptor is invalid, then it indicates that the next-level
    // page table is not present yet. Hence, we should allocate the next-level
    // page table now.
    if (PD_INVALID(pd)) [[unlikely]] {
      Kernel::panic("copy_page_frame(): page directory / entry doesn't exist?");
    }

    // Advance to the next level page table.
    next_level_pt_addr = pt[pt_index] & PAGE_MASK;
    pt = reinterpret_cast<pagetable_t *>(next_level_pt_addr);
  }

  // We've reached the last-level page table. Duplicate the page frame and update PTE.
  pt_index = pt_indices[PAGE_TABLE_DEPTH - 1];

  void *old_page_frame = reinterpret_cast<void *>(pt[pt_index] & PAGE_MASK);
  auto &mm = MemoryManager::the();

  if (mm.get_page_ref_count(old_page_frame) == 1) {
    //pt[pt_index] &= ~PD_COW_PAGE;
    //pt[pt_index] |= USER_PAGE_RWX;
    size_t addr = pt[pt_index] & PAGE_MASK;
    pt[pt_index] = addr | USER_PAGE_RWX;
  } else {
    void *new_page_frame = get_free_page(true);
    memcpy(new_page_frame, old_page_frame, PAGE_SIZE);

    pt[pt_index] = reinterpret_cast<size_t>(new_page_frame) | USER_PAGE_RWX;

    MemoryManager::the().dec_page_ref_count(old_page_frame);
    MemoryManager::the().inc_page_ref_count(new_page_frame);
  }
}

void VMMap::dfs_kfree_page(pagetable_t *pt, const size_t level) const {
  // `level` begins from 0 (PGD), and goes all the way down to
  // 1 (PUD), 2 (PMD), 3 (PTE). We'll stop at PTE and free all
  // the pages that are pointed to by this PTE.
  if (level == 3) {
    for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
      if (PD_INVALID(pt[i])) {
        continue;
      }

      size_t addr = pt[i] & PAGE_MASK;
      void *p_addr = reinterpret_cast<void *>(addr);

      if (MemoryManager::the().dec_page_ref_count(p_addr) == 0) {
        kfree(p_addr);
      }
    }
    return;
  }

  // DFS
  for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
    if (PD_INVALID(pt[i])) {
      continue;
    }
    dfs_kfree_page(reinterpret_cast<pagetable_t *>(pt[i] & PAGE_MASK), level + 1);
    kfree(reinterpret_cast<void *>(pt[i] & PAGE_MASK));
  }
}

void VMMap::dfs_copy_page_tables(pagetable_t *pt_old, pagetable_t *pt_new,
                                 const size_t level) const {
  // `level` begins from 0 (PGD), and goes all the way down to
  // 1 (PUD), 2 (PMD), 3 (PTE). We'll stop at PTE and duplicate
  // all the underlying page frames.
  if (level == 3) {
    for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
      if (PD_INVALID(pt_old[i])) {
        continue;
      }

      // Let both parent and child share this page frame for now.
      size_t page_frame_addr = pt_old[i] & PAGE_MASK;
      pt_old[i] = pt_new[i] = page_frame_addr | USER_PAGE_RX | PD_COW_PAGE;

      void *p_addr = reinterpret_cast<void *>(page_frame_addr);
      MemoryManager::the().inc_page_ref_count(p_addr);
    }
    return;
  }

  // DFS
  for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
    if (PD_INVALID(pt_old[i])) {
      continue;
    }

    // Duplicate the page frame used by this page table.
    auto old_page_frame = reinterpret_cast<pagetable_t *>(pt_old[i] & PAGE_MASK);
    auto new_page_frame = reinterpret_cast<pagetable_t *>(get_free_page(true));
    memset(new_page_frame, 0, PAGE_SIZE);

    pt_new[i] = reinterpret_cast<size_t>(new_page_frame) | PD_TABLE;
    dfs_copy_page_tables(old_page_frame, new_page_frame, level + 1);
  }
}

}  // namespace valkyrie::kernel

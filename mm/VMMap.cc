// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/VMMap.h>

#include <dev/Console.h>
#include <libs/CString.h>
#include <mm/MemoryManager.h>

#define PAGE_TABLE_DEPTH  4
#define NR_ENTRIES_PER_PT (PAGE_SIZE / sizeof(size_t))

namespace valkyrie::kernel {

VMMap::VMMap()
    : _pgd(reinterpret_cast<page_table_t*>(get_free_page())) {

  if (!_pgd) [[unlikely]] {
    printk("error: unable to allocate pgd\n");
    return;
  }

  memset(_pgd, 0, PAGE_SIZE);
}

VMMap::~VMMap() {
  dfs_kfree_page(_pgd, 0);
}

void VMMap::dfs_kfree_page(page_table_t* pt, const size_t level) const {
  // `level` begins from 0 (PGD), and goes all the way down to
  // 1 (PUD), 2 (PMD), 3 (PTE). We'll stop at PTE and free all
  // the pages that are pointed to by this PTE.
  if (level == 3) {
    for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
      if (PD_INVALID(pt[i])) {
        continue;
      }
      kfree(reinterpret_cast<void*>(pt[i] & PAGE_MASK));
    }
    return;
  }

  // DFS
  for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
    if (PD_INVALID(pt[i])) {
      continue;
    }
    dfs_kfree_page(reinterpret_cast<page_table_t*>(pt[i] & PAGE_MASK), level + 1);
  }

  // Finally free PGD page frame
  kfree(_pgd);
}


void VMMap::map(const size_t vaddr,
                const void* paddr,
                const size_t attr) const {
  printk("VMMap::map: vaddr = 0x%x, paddr = 0x%x\n", vaddr, paddr);

  // Extract the page table indices from `vaddr`.
  size_t pt_indices[PAGE_TABLE_DEPTH] = {
    PGD_INDEX(vaddr),
    PUD_INDEX(vaddr),
    PMD_INDEX(vaddr),
    PTE_INDEX(vaddr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  page_table_t* pt = _pgd;    // current page table
  size_t pt_index;            // page table index
  size_t pd;                  // page descriptor
  size_t next_level_pt_addr;  // addr of next level page table

  for (int i = 0; i < PAGE_TABLE_DEPTH - 1; i++) {
    pt_index = pt_indices[i];
    pd = pt[pt_index];

    // If this page descriptor is invalid, then
    // it indicates that the next-level page table
    // is not present yet. Hence, we should allocate
    // the next-level page table now.
    if (PD_INVALID(pd)) {
      void* page_frame = get_free_page();
      memset(page_frame, 0, PAGE_SIZE);
      next_level_pt_addr = reinterpret_cast<size_t>(page_frame);
      pt[pt_index] = next_level_pt_addr | PD_TABLE;
    } else {
      next_level_pt_addr = pt[pt_index] & PAGE_MASK;
    }

    // Advance to the next level page table.
    pt = reinterpret_cast<page_table_t*>(next_level_pt_addr);
  }

  // We've reached the last-level page table.
  pt_index = pt_indices[PAGE_TABLE_DEPTH - 1];
  pd = pt[pt_index];

  if (!PD_INVALID(pd)) [[unlikely]] {
    printk("VMMap::map: vaddr: 0x%x has already been mapped to paddr: 0x%x\n",
           vaddr, paddr);
    return;
  }

  pt[pt_index] = reinterpret_cast<size_t>(paddr) | attr | PD_PAGE;
}

void VMMap::unmap(const size_t vaddr) const {
  printk("VMMap::unmap: vaddr = 0x%x\n", vaddr);

  // Extract the page table indices from `vaddr`.
  size_t pt_indices[PAGE_TABLE_DEPTH] = {
    PGD_INDEX(vaddr),
    PUD_INDEX(vaddr),
    PMD_INDEX(vaddr),
    PTE_INDEX(vaddr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  page_table_t* pt = _pgd;    // current page table
  size_t pt_index;            // page table index
  size_t pd;                  // page descriptor
  size_t next_level_pt_addr;  // addr of next level page table

  for (int i = 0; i < PAGE_TABLE_DEPTH - 1; i++) {
    pt_index = pt_indices[i];
    pd = pt[pt_index];

    // If this page descriptor is invalid, then
    // it indicates that the next-level page table
    // is not present yet. Hence, we can return immediately.
    if (PD_INVALID(pd)) {
      printk("VMMap::map: vaddr: 0x%x has not been mapped yet...\n", vaddr);
      return;
    }

    // Advance to the next level page table.
    next_level_pt_addr = pt[pt_index] & PAGE_MASK;
    pt = reinterpret_cast<page_table_t*>(next_level_pt_addr);
  }

  // We've reached the last-level page table.
  pt_index = pt_indices[PAGE_TABLE_DEPTH - 1];
  pd = pt[pt_index];

  if (PD_INVALID(pd)) [[unlikely]] {
    printk("VMMap::map: vaddr: 0x%x has not been mapped yet...\n", vaddr);
    return;
  }

  pt[pt_index] = 0;
}

void VMMap::copy_from(const VMMap& r) const {
  printk("cloning vmmap...\n");
  dfs_copy_page(r._pgd, _pgd, 0);
}

void VMMap::dfs_copy_page(const page_table_t* pt_old,
                          page_table_t* pt_new,
                          const size_t level) const {
  // `level` begins from 0 (PGD), and goes all the way down to
  // 1 (PUD), 2 (PMD), 3 (PTE). We'll stop at PTE and duplicate
  // all the underlying page frames.
  if (level == 3) {
    for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
      if (PD_INVALID(pt_old[i])) {
        continue;
      }
      // Duplicate page frames.
      void* old_page_frame = reinterpret_cast<void*>(pt_old[i] & PAGE_MASK);
      void* new_page_frame = get_free_page();
      memcpy(new_page_frame, old_page_frame, PAGE_MASK);

      pt_new[i] = reinterpret_cast<size_t>(new_page_frame) | PD_PAGE;
    }
    return;
  }

  // DFS
  for (size_t i = 0; i < NR_ENTRIES_PER_PT; i++) {
    if (PD_INVALID(pt_old[i])) {
      continue;
    }

    // Duplicate the page frame used by this page table.
    auto old_page_frame = reinterpret_cast<page_table_t*>(pt_old[i] & PAGE_MASK);
    auto new_page_frame = reinterpret_cast<page_table_t*>(get_free_page());

    pt_new[i] = reinterpret_cast<size_t>(new_page_frame) | PD_TABLE;
    dfs_copy_page(old_page_frame, new_page_frame, level + 1);
  }
}

size_t* VMMap::get_pgd() const {
  return _pgd;
}

}  // namespace valkyrie::kernel

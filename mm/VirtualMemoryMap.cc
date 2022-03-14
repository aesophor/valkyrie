// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/VirtualMemoryMap.h>

#include <CString.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <mm/MemoryManager.h>

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

void VMMap::copy_from(const VMMap &r) const {
  // 1. Copy the page frames of page tables.
  // 2. Mark PTEs of both child & parent to read-only even for original read-write pages.
  dfs_copy_page_tables(r._pgd, _pgd, 0);
}

VMMap::pagetable_t *VMMap::walk(const size_t v_addr, bool create_pte) const {
  // Extract the page table indices from `v_addr`.
  size_t pt_indices[page_table_depth] = {
      PGD_INDEX(v_addr),
      PUD_INDEX(v_addr),
      PMD_INDEX(v_addr),
      PTE_INDEX(v_addr),
  };

  // Follow PGD -> PUD -> PMD -> PTE
  pagetable_t *pt = _pgd;     // current page table
  size_t pt_index;            // page table index
  size_t next_level_pt_addr;  // addr of next level page table

  for (size_t i = 0; i < page_table_depth - 1; i++) {
    pt_index = pt_indices[i];

    // If this page descriptor is invalid, then it indicates that the next-level
    // page table is not present yet.
    if (!PD_INVALID(pt[pt_index])) {
      next_level_pt_addr = pt[pt_index] & PD_PAGE_MASK;
    } else if (!create_pte) {
      return nullptr;
    } else {
      void *page_frame = get_free_page(/*physical=*/true);
      memset(page_frame, 0, PAGE_SIZE);
      next_level_pt_addr = reinterpret_cast<size_t>(page_frame);
      pt[pt_index] = next_level_pt_addr | PD_TABLE;
    }

    // Advance to the next level page table.
    pt = reinterpret_cast<pagetable_t *>(next_level_pt_addr);
  }

  // We've reached the last-level page table.
  pt_index = pt_indices[page_table_depth - 1];
  return &pt[pt_index];
}

void VMMap::map(const size_t v_addr, const void *const p_addr, size_t attr) const {
#ifdef DEBUG
  printk("[%s] VMMap::map: v_addr = 0x%p, p_addr = 0x%p\n", Task::current()->get_name(),
         v_addr, p_addr);
#endif

  pagetable_t *pte = walk(v_addr, /*create_pte=*/true);

  if (!PD_INVALID(*pte)) [[unlikely]] {
    Kernel::panic("VMMap::map: v_addr: 0x%p has already been mapped to p_addr: 0x%p\n", v_addr,
                  p_addr);
  }

  // Has write permission, so make it copy-on-write.
  if (!(attr & PD_RDONLY)) {
    attr |= PD_COW_PAGE;
  }

  *pte = reinterpret_cast<size_t>(p_addr) | attr;
  MemoryManager::the().inc_page_ref_count(p_addr);
}

void VMMap::unmap(const size_t v_addr) const {
#ifdef DEBUG
  printk("[%s] VMMap::unmap: v_addr = 0x%p\n", Task::current()->get_name(), v_addr);
#endif

  pagetable_t *pte = walk(v_addr);

  if (!pte) [[unlikely]] {
    Kernel::panic("VMMap::map: v_addr: 0x%p has not been mapped yet...\n", v_addr);
  }

  size_t page_frame_addr = reinterpret_cast<size_t>(*pte & PD_PAGE_MASK);
  void *p_addr = reinterpret_cast<void *>(page_frame_addr);

  *pte = 0;
  MemoryManager::the().dec_page_ref_count(p_addr);
}

bool VMMap::is_cow_page(const size_t v_addr) const {
  if (!Page::is_aligned(v_addr)) [[unlikely]] {
    Kernel::panic("VMMap::walk(): v_addr unaligned to page boundary: 0x%p\n", v_addr);
  }

  pagetable_t *pte = walk(v_addr);

  return pte && *pte & PD_COW_PAGE;
}

void *VMMap::get_physical_address(const size_t v_addr) const {
  pagetable_t *pte = walk(v_addr);

  if (!pte) [[unlikely]] {
    // printk("warning: cannot get physical address for virtual address: 0x%p\n", v_addr);
    return nullptr;
  }

  return reinterpret_cast<void *>((*pte & PD_PAGE_MASK) + PAGE_OFFSET(v_addr));
}

size_t VMMap::get_unmapped_area(size_t len) const {
  // XXX: We should keep track of mapped areas using a tree map.
  return 0;
}

void VMMap::copy_page_frame(const size_t v_addr) const {
  pagetable_t *pte = walk(v_addr);

  if (!pte) [[unlikely]] {
    Kernel::panic("copy_page_frame(): page directory / entry doesn't exist?");
  }

  auto &mm = MemoryManager::the();
  void *old_page_frame = reinterpret_cast<void *>(*pte & PD_PAGE_MASK);
  size_t old_attr = *pte & PD_ATTR_MASK;

  if (mm.get_page_ref_count(old_page_frame) == 1) {
    *pte &= ~PD_COW_PAGE;
    *pte &= ~PD_RDONLY;
  } else {
    void *new_page_frame = get_free_page(true);
    memcpy(new_page_frame, old_page_frame, PAGE_SIZE);

    *pte = reinterpret_cast<size_t>(new_page_frame) | old_attr;
    *pte &= ~PD_RDONLY;

    mm.dec_page_ref_count(old_page_frame);
    mm.inc_page_ref_count(new_page_frame);
  }
}

void VMMap::dfs_kfree_page(pagetable_t *pt, const size_t level) const {
  // `level` begins from 0 (PGD), and goes all the way down to
  // 1 (PUD), 2 (PMD), 3 (PTE). We'll stop at PTE and free all
  // the pages that are pointed to by this PTE.
  for (size_t i = 0; i < nr_entries_per_pt; i++) {
    if (PD_INVALID(pt[i])) {
      continue;
    }

    if (level == 3) {
      size_t addr = pt[i] & PD_PAGE_MASK;
      void *p_addr = reinterpret_cast<void *>(addr);

      if (MemoryManager::the().dec_page_ref_count(p_addr) == 0) {
        kfree(p_addr);
      }
    } else {
      dfs_kfree_page(reinterpret_cast<pagetable_t *>(pt[i] & PD_PAGE_MASK), level + 1);
      kfree(reinterpret_cast<void *>(pt[i] & PD_PAGE_MASK));
    }
  }
}

void VMMap::dfs_copy_page_tables(pagetable_t *pt_old, pagetable_t *pt_new,
                                 const size_t level) const {
  // `level` begins from 0 (PGD), and goes all the way down to
  // 1 (PUD), 2 (PMD), 3 (PTE). We'll stop at PTE and duplicate
  // all the underlying page frames.
  for (size_t i = 0; i < nr_entries_per_pt; i++) {
    if (PD_INVALID(pt_old[i])) {
      continue;
    }

    if (level == 3) {
      // Let both parent and child share this page frame for now.
      size_t page_frame_addr = pt_old[i] & PD_PAGE_MASK;
      pt_old[i] = pt_new[i] = page_frame_addr | USER_PAGE_RX | PD_COW_PAGE;

      void *p_addr = reinterpret_cast<void *>(page_frame_addr);
      MemoryManager::the().inc_page_ref_count(p_addr);

    } else {
      // Duplicate the page frame used by this page table.
      auto old_page_frame = reinterpret_cast<pagetable_t *>(pt_old[i] & PD_PAGE_MASK);
      auto new_page_frame = reinterpret_cast<pagetable_t *>(get_free_page(true));
      memset(new_page_frame, 0, PAGE_SIZE);

      pt_new[i] = reinterpret_cast<size_t>(new_page_frame) | PD_TABLE;
      dfs_copy_page_tables(old_page_frame, new_page_frame, level + 1);
    }
  }
}

}  // namespace valkyrie::kernel

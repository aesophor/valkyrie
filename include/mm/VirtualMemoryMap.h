// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// VirtualMemoryMap (VMMap) - userspace process virtual memory map

#ifndef VALKYRIE_VIRTUAL_MEMORY_MAP_H_
#define VALKYRIE_VIRTUAL_MEMORY_MAP_H_

#include <List.h>
#include <TypeTraits.h>

#include <mm/Page.h>
#include <mm/mmu.h>

namespace valkyrie::kernel {

class VMMap final {
  MAKE_NONCOPYABLE(VMMap);
  MAKE_NONMOVABLE(VMMap);

  using pagetable_t = size_t;

 public:
  VMMap();
  ~VMMap();

  // Clear page table.
  void reset() const;

  // Deep copy the entire page table and the underlying page frames.
  void copy_from(const VMMap &r) const;

  // Walks the page table and returns the PTE of the given `v_addr`.
  // If `create_pte` is true, then the missing PTEs will be created as necessary.
  pagetable_t *walk(const size_t v_addr, bool create_pte = false) const;

  // Maps a single page.
  // @v_addr: specifies the base virtual address of the target page.
  // @p_addr: specifies the base physical address of the page frame.
  // @attr:   page attribute; see include/mm/mmu.h
  void map(const size_t v_addr, const void *p_addr, const size_t attr = USER_PAGE_RWX) const;

  // Unmaps a single page.
  void unmap(const size_t v_addr) const;

  // Is copy-on-write page?
  bool is_cow_page(const size_t v_addr) const;

  // Gets the physical address from a virtual address by parsing the page table.
  void *get_physical_address(const void *const v_addr) const;

  void copy_page_frame(const size_t v_addr) const;

  [[nodiscard]] size_t *get_pgd() const {
    return _pgd;
  }

 private:
  // TODO: maybe refactor this with STL Function<>
  void dfs_kfree_page(pagetable_t *pt, const size_t level) const;

  void dfs_copy_page_tables(pagetable_t *pt_old, pagetable_t *pt_new,
                            const size_t level) const;

  static constexpr const size_t page_table_depth = 4;
  static constexpr const size_t nr_entries_per_pt = PAGE_SIZE / sizeof(size_t);

  pagetable_t *const _pgd;  // points to PGD's page frame
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VIRTUAL_MEMORY_MAP_H_

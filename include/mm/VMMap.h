// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// VMMap - userspace process virtual memory map

#ifndef VALKYRIE_VM_MAP_H_
#define VALKYRIE_VM_MAP_H_

#include <List.h>
#include <mm/mmu.h>
#include <mm/Page.h>

#define PAGE_RWX ((MAIR_IDX_NORMAL_NOCACHE << 2) | PD_ACCESS | PD_KERNEL_USER | PD_PAGE)

namespace valkyrie::kernel {

class VMMap final {
  using page_table_t = size_t;

 public:
  VMMap();
  ~VMMap();
  VMMap(const VMMap&) = delete;
  VMMap(VMMap&&) = delete;
  VMMap& operator =(const VMMap&) = delete;
  VMMap& operator =(VMMap&&) = delete;


  // Maps a single page.
  // Returns `paddr` to the caller.
  //
  // * `v_addr`: specifies the base virtual address of the target page.
  // * `p_addr`: specifies the base physical address of the page frame.
  // * `attr`:   page attribute; see include/mm/mmu.h
  void map(const size_t v_addr,
           const void* p_addr,
           const size_t attr = PAGE_RWX) const;

  // Unmaps a single page.
  void unmap(const size_t v_addr) const;

  // Clear page table.
  void reset() const;

  // Gets the physical address from a virtual address
  // by parsing the page table.
  void* get_physical_address(const void* const v_addr) const;

  // Deep copy the entire page table and the underlying page frames.
  // Returns the new _pgd.
  void copy_from(const VMMap& r) const;

  [[nodiscard]] size_t* get_pgd() const;

 private:
  // TODO: maybe refactor this with STL Function<>
  void dfs_kfree_page(page_table_t* pt, const size_t level) const;

  void dfs_copy_page(const page_table_t* pt_old,
                     page_table_t* pt_new,
                     const size_t level) const;

  page_table_t* const _pgd;  // points to PGD's page frame
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VM_MAP_H_

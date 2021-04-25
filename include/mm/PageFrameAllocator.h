// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PAGE_FRAME_ALLOCATOR_H_
#define VALKYRIE_PAGE_FRAME_ALLOCATOR_H_

#include <mm/BuddyAllocator.h>

namespace valkyrie::kernel {

// The entire physical address space is divided into a number of "zones",
// where each zone is managed by exactly one buddy allocator.
class PageFrameAllocator {
 public:
  PageFrameAllocator(const size_t begin, size_t size);
  ~PageFrameAllocator() = default;

  void* allocate(size_t requested_size);
  void  deallocate(void* p);
  void  dump_memory_map() const;

  void* allocate_one_page_frame();

  static size_t get_block_header_size();

 private:
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PAGE_FRAME_ALLOCATOR_H_

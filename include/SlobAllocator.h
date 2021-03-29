// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLOB_ALLOCATOR_H_
#define VALKYRIE_SLOB_ALLOCATOR_H_

#include <PageFrameAllocator.h>

#define NUM_SUPPORTED_CHUNK_SIZES 10

namespace valkyrie::kernel {

class SlobAllocator {
 public:
  SlobAllocator(PageFrameAllocator* page_frame_allocator);
  ~SlobAllocator() = default;

  void* allocate(size_t requested_size);
  void  deallocate(void* p);
  void  dump_slob_info() const;

 private:
  int size_to_free_list_idx(const size_t size);

  struct Slob {
    Slob* next;
    size_t size;
  };

  static const size_t _supported_chunk_sizes[NUM_SUPPORTED_CHUNK_SIZES];

  PageFrameAllocator* _page_frame_allocator;
  Slob* _free_lists[NUM_SUPPORTED_CHUNK_SIZES];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLOB_ALLOCATOR_H_

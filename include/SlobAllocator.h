// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLOB_ALLOCATOR_H_
#define VALKYRIE_SLOB_ALLOCATOR_H_

#include <PageFrameAllocator.h>

#define CHUNK_SIZE 32

#define SLOB_MAX_ORDER 6

namespace valkyrie::kernel {

class SlobAllocator {
 public:
  SlobAllocator(PageFrameAllocator* page_frame_allocator);
  ~SlobAllocator() = default;

  void* allocate(size_t requested_size);
  void  deallocate(void* p);
  void  dump_slob_info() const;

 private:
  struct Slob {
    Slob* next;
    int64_t order;
  };

  Slob* split_from_top_chunk(size_t requested_size);
  bool  is_top_chunk_used_up() const;

  void free_list_del_head(Slob* chunk);
  void free_list_add_head(Slob* chunk);

  int size_to_order(const size_t size);

  PageFrameAllocator* _page_frame_allocator;
  Slob* _free_lists[SLOB_MAX_ORDER];
  void* _top_chunk;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLOB_ALLOCATOR_H_

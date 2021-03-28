// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLAB_ALLOCATOR_H_
#define VALKYRIE_SLAB_ALLOCATOR_H_

#include <PageFrameAllocator.h>

namespace valkyrie::kernel::io {

class SlabAllocator {
 public:
  SlabAllocator();
  ~SlabAllocator() = default;

 private:
  PageFrameAllocator& _page_frame_allocator;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLAB_ALLOCATOR_H_

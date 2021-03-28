// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLAB_ALLOCATOR_H_
#define VALKYRIE_SLAB_ALLOCATOR_H_

#include <PageFrameAllocator.h>

namespace valkyrie::kernel {

class SlabAllocator {
 public:
  SlabAllocator();
  ~SlabAllocator() = default;

 private:
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLAB_ALLOCATOR_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLOB_ALLOCATOR_H_
#define VALKYRIE_SLOB_ALLOCATOR_H_

#include <PageFrameAllocator.h>

namespace valkyrie::kernel {

class SlobAllocator {
 public:
  SlobAllocator();
  ~SlobAllocator() = default;

 private:

};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLOB_ALLOCATOR_H_

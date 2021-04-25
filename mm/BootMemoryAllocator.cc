// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/BootMemoryAllocator.h>

#include <libs/Math.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

BootMemoryAllocator::BootMemoryAllocator()
    : _bitmap() {
  // Mark
}


void* BootMemoryAllocator::allocate(size_t requested_size) {
  requested_size = round_up_to_multiple_of_n(requested_size, PAGE_SIZE);

  // Linear search the bitmap.
  for (auto& bit : _bitmap) {

  }
}


void BootMemoryAllocator::mark_region_free(const size_t begin,
                                           const size_t end) {
  
}

void BootMemoryAllocator::mark_region_allocated(const size_t begin,
                                                const size_t end) {

}

}  // namespace valkyrie::kernel

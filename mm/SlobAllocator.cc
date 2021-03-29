// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <SlobAllocator.h>

#include <Console.h>
#include <Kernel.h>
#include <Math.h>

namespace valkyrie::kernel {

const size_t SlobAllocator::_supported_chunk_sizes[NUM_SUPPORTED_CHUNK_SIZES]
  = {16, 32, 48, 64, 96, 128, 256, 512, 1024, 2048};


SlobAllocator::SlobAllocator(PageFrameAllocator* page_frame_allocator)
    : _page_frame_allocator(page_frame_allocator),
      _free_lists() {
  // Request a page frame.
  void* page_frame = _page_frame_allocator->allocate_one_page_frame();

  // Partition the page frame into several chunks of different sizes.

}


void* SlobAllocator::allocate(size_t requested_size) {

}

void SlobAllocator::deallocate(void* p) {

}

void SlobAllocator::dump_slob_info() const {
  puts("--- dumping slob free lists ---");

  for (int i = 0; i < NUM_SUPPORTED_CHUNK_SIZES; i++) {
    printf("_free_lists[%d]: ", i);
    Slob* ptr = _free_lists[i];
    while (ptr) {
      printf("[%d] -> ", ptr->size);
      ptr = ptr->next;
    }
    printf("[nil]\n");
  }

  puts("--- end dumping slob free lists ---");
}


int SlobAllocator::size_to_free_list_idx(const size_t size) {
  const int len = sizeof(_supported_chunk_sizes) /
                  sizeof(_supported_chunk_sizes[0]);

  for (int i = 0; i < len; i++) {
    if (_supported_chunk_sizes[i] == size) {
      return i;
    }
  }

  Kernel::panic("kernel heap corrupted: "
                "size %d is not supported by slob allocator\n", size);
  return -1;
}

}  // namespace valkyrie::kernel

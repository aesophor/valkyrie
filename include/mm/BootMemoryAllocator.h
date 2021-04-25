// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// In general purpose operating systems, the amount of physical memory
// is determined at runtime. Hence, a kernel needs to dynamically
// allocate its page frame array for its page frame allocator.
// The page frame allocator then depends on dynamic memory allocation.
// The dynamic memory allocator depends on the page frame allocator.
// This introduces the chicken or the egg problem. To break the dilemma,
// you need a dedicated allocator during startup time.
//
// The design of the startup allocator is quite simple. Just implement
// a dynamic memory allocator not based on the page allocator.
// It records the start address and size of the allocated and
// reserved blocks in a statically allocated array.
// If there are not many memory holes in the physical memory,
// it can bookkeep with a minimum number of entries.
// 
// Your startup allocator should be able to reserve memory for
// the buddy system, kernel, initramfs, etc. In the end,
// it hands the physical memory to the buddy system.
// The buddy system should mark the reserved segment as allocated.

#ifndef VALKYRIE_BOOT_MEMORY_ALLOCATOR_H_
#define VALKYRIE_BOOT_MEMORY_ALLOCATOR_H_

#include <Types.h>

#define BITMAP_SIZE 32768  /* 0x0 ~ 0x8000000 */

namespace valkyrie::kernel {

class BootMemoryAllocator {
 public:
  BootMemoryAllocator();
  ~BootMemoryAllocator() = default;

  void* allocate(size_t requested_size);
  void mark_region_allocated(const size_t begin, const size_t end);

 private:
  void mark_region_free(const size_t begin, const size_t end);

  // Each bit in the `_bitmap` represents a page frame.
  // 0: free, 1: allocated
  bool _bitmap[BITMAP_SIZE];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_BOOT_MEMORY_ALLOCATOR_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <PageFrameAllocator.h>

#include <Console.h>
#include <Kernel.h>
#include <Math.h>

namespace valkyrie::kernel {

PageFrameAllocator::PageFrameAllocator()
    : _frame_array(),
      _frame_array_size(sizeof(_frame_array) / sizeof(_frame_array[0])),
      _free_lists() {
  for (auto& entry : _frame_array) {
    entry = DONT_ALLOCATE;
  }
  _frame_array[0] = sqrt(_frame_array_size);

  // The last free list should point to HEAP_BEGIN,
  // while all the other free list should be nullptr.
  int idx = get_free_list_index(HEAP_END - HEAP_BEGIN);
  _free_lists[idx] = reinterpret_cast<PageFrameHeader*>(HEAP_BEGIN);
  _free_lists[idx]->size = HEAP_END - HEAP_BEGIN;
  _free_lists[idx]->next = nullptr;
}


void* PageFrameAllocator::allocate(size_t requested_size) {
  // For each allocation request x, add the header size to x and
  // raise that value to a power of 2 s.t. x >= the original requested_size.
  requested_size += sizeof(PageFrameHeader);
  int idx = get_free_list_index(requested_size);

  if (idx >= MAX_ORDER) {
    Kernel::panic("idx >= MAX_ORDER\n");
  }

  printf("allocating physical memory of %d bytes...\n", requested_size);
  printf("searching free page frames from _free_lists[%d]...\n", idx);

  // If there's an exact-fit page frame from the free list,
  // then remove it from the free list and return that page frame.
  PageFrameHeader* victim = _free_lists[idx];
  while (victim) {
    if (victim->size == requested_size) {
      printf("hit!\n");
      mark_page_frame_as_allocated(victim);
      break;
    }
    victim = victim->next;
  }

  dump_memory_map();
}

void PageFrameAllocator::deallocate(void* p) {
  if (!p) {
    return;
  }

  
}


void* PageFrameAllocator::get_page_frame_by_index(const size_t index) const {
  return reinterpret_cast<void*>(HEAP_BEGIN + PAGE_SIZE * index);
}

int PageFrameAllocator::get_page_frame_index(PageFrameHeader* header) const {
  return (reinterpret_cast<size_t>(header) - HEAP_BEGIN) / PAGE_SIZE;
}

void PageFrameAllocator::mark_page_frame_as_allocated(PageFrameHeader* header) {
  // Update the frame array.
  int index = get_page_frame_index(header);
  int len = pow(2, header->size);

  printf("index = %d\n", index);

  for (int i = 0; i < len; i++) {
    _frame_array[index + i] = ALLOCATED;
  }
}

void PageFrameAllocator::dump_memory_map() const {
  puts("\n~~~ dumping kernel heap~~~");

  for (int i = 0; i < _frame_array_size; i++) {
    if (_frame_array[i] == DONT_ALLOCATE) {
      // Do nothing.
    } else if (_frame_array[i] == ALLOCATED) {
      printf("<%d>: allocated page frame\n", i);
    } else {
      printf("<%d>: free page frame\n", i);
    }
  }
  puts("~~~ end dumping kernel heap~~~\n");
}


size_t PageFrameAllocator::get_free_list_index(const size_t x) {
  int exponent = 0;
  int value = PAGE_SIZE;

  while (exponent < MAX_ORDER) {
    if (value >= x) {
      break;
    }
    value <<= 1;
    ++exponent;
  }

  return exponent;
}

}  // namespace valkyrie::kernel

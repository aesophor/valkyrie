// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <PageFrameAllocator.h>

#include <Console.h>
#include <Kernel.h>
#include <Math.h>
#include <Utility.h>

namespace valkyrie::kernel {

PageFrameAllocator::PageFrameAllocator()
    : _frame_array(),
      _frame_array_size(sizeof(_frame_array) / sizeof(_frame_array[0])),
      _free_lists() {
  for (auto& entry : _frame_array) {
    entry = static_cast<int8_t>(DONT_ALLOCATE);
  }
  _frame_array[0] = sqrt(_frame_array_size);

  int order = size_to_order(HEAP_END - HEAP_BEGIN);
  _free_lists[order] = reinterpret_cast<Block*>(HEAP_BEGIN);
  _free_lists[order]->order = _frame_array[0];
  _free_lists[order]->next = nullptr;
}


void* PageFrameAllocator::allocate(size_t requested_size) {
  // For each allocation request x, add the block size to x and
  // raise that value to a power of 2 s.t. x >= the original requested_size.
  requested_size += sizeof(Block);
  int order = size_to_order(requested_size);

  if (order >= MAX_ORDER) {
    Kernel::panic("order >= MAX_ORDER\n");
  }

  printf("allocating physical memory of %d bytes...\n", requested_size);
  printf("searching free block from _free_lists[%d]...\n", order);

  // If there's an exact-fit free block from the free list,
  // then remove it from the free list and return that free block.
  Block* victim = _free_lists[order];

  if (victim && victim->order == requested_size / PAGE_SIZE) {
    printf("freelist hit!\n");
    mark_block_as_allocated(victim);
    free_list_del_head(victim);
    dump_memory_map();
    return victim + 1;  // skip the header 
  }

  printf("no existing free block available\n");

  // Search larger free blocks.
  for (; order <= MAX_ORDER; order++) {
    if (_free_lists[order]) {
      victim = _free_lists[order];
      printf("found a larger block with order %d\n", victim->order);
      break;
    }
  }

  if (order >= MAX_ORDER) {
    Kernel::panic("out of memory\n");
  }

  // Recursively divide the victim free block in half,
  // and update _free_lists until we've found an exact fit.
  victim = split_block(victim, size_to_order(requested_size));
  mark_block_as_allocated(victim);

  dump_memory_map();
  return victim + 1;  // skip the header
}

void PageFrameAllocator::deallocate(void* p) {
  if (!p) {
    return;
  }

  
}

void PageFrameAllocator::dump_memory_map() const {
  puts("--- dumping kernel heap ---");

  for (int i = 0; i < _frame_array_size; i++) {
    if (_frame_array[i] == static_cast<int8_t>(DONT_ALLOCATE)) {
      // Do nothing.
    } else if (_frame_array[i] == static_cast<int8_t>(ALLOCATED)) {
      printf("<page frame #%d>: allocated page frame\n", i);
    } else {
      printf("<page frame #%d>: free page frame\n", i);
    }
  }

  puts("");

  for (int i = 0; i < MAX_ORDER; i++) {
    printf("_free_lists[%d]: ", i);
    Block* ptr = _free_lists[i];
    while (ptr) {
      printf("[%d] ->", ptr->order);
      ptr = ptr->next;
    }
    printf("[nil]\n");
  }

  puts("--- end dumping kernel heap ---");
}


int PageFrameAllocator::get_page_frame_index(const Block* block) const {
  return (reinterpret_cast<size_t>(block) - HEAP_BEGIN) / PAGE_SIZE;
}

void PageFrameAllocator::mark_block_as_allocated(const Block* block) {
  int idx = get_page_frame_index(block);
  int len = pow(2, block->order);

  for (int i = 0; i < len; i++) {
    _frame_array[idx + i] = static_cast<int8_t>(ALLOCATED);
  }
}

void PageFrameAllocator::mark_block_as_allocatable(const Block* block) {
  int idx = get_page_frame_index(block);
  int len = pow(2, block->order);

  _frame_array[idx] = static_cast<int8_t>(block->order);
  for (int i = 1; i < len; i++) {
    _frame_array[idx + i] = static_cast<int8_t>(DONT_ALLOCATE);
  }
}


void PageFrameAllocator::free_list_del_head(Block* block) {
  if (!_free_lists[block->order]) {
    return;
  }
  _free_lists[block->order] = _free_lists[block->order]->next;
  block->next = nullptr;
}

void PageFrameAllocator::free_list_add_head(Block* block) {
  // If the list is empty
  if (!_free_lists[block->order]) {
    _free_lists[block->order] = block;
    block->next = nullptr;
  } else {
    block->next = _free_lists[block->order];
    _free_lists[block->order] = block;
  }
}

PageFrameAllocator::Block* PageFrameAllocator::split_block(Block* block,
                                                           const int target_order) {
  if (!block) {
    Kernel::panic("kernel heap corrupted (block == nullptr)\n");
  }

  if (block->order < 0) {
    Kernel::panic("kernel heap corrupted (block->order < 0)\n");
  }


  printf("---------------------------------\n");
  printf("comparing block order... %d vs %d\n", block->order, target_order);

  // Otherwise, we need to split it recursively.
  free_list_del_head(block);

  if (block->order == target_order) {
    return block;
  }

  // Update block headers
  block->order--;
  Pair<Block*, Block*> buddies = {block, get_buddy(block)};
  buddies.second->order = buddies.first->order;
  printf("buddies: b1 = 0x%x, b2 = 0x%x\n", buddies.first, buddies.second);

  // Add buddy2 to the free list.
  free_list_add_head(buddies.second);
  free_list_add_head(buddies.first);

  // Update _frame_array.
  mark_block_as_allocatable(buddies.first);
  mark_block_as_allocatable(buddies.second);

  // Continue splitting buddy1.
  return split_block(buddies.first, target_order);
}

PageFrameAllocator::Block* PageFrameAllocator::get_buddy(Block* block) {
  printf("block order = %d\n", block->order);
  const size_t b1 = reinterpret_cast<size_t>(block);
  const size_t b2 = b1 ^ (1 << (block->order)) * PAGE_SIZE;
  return reinterpret_cast<Block*>(b2);
}

int PageFrameAllocator::size_to_order(const size_t size) {
  // e.g., 4096  -> 0
  //       8192  -> 1
  //       16384 -> 2
  return log2(size / PAGE_SIZE);
}

}  // namespace valkyrie::kernel

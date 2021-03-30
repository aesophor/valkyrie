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
  const int order = size_to_order(HEAP_END - HEAP_BEGIN);

  for (auto& entry : _frame_array) {
    entry = static_cast<int8_t>(DONT_ALLOCATE);
  }
  _frame_array[0] = static_cast<int8_t>(order);

  _free_lists[order] = reinterpret_cast<Block*>(HEAP_BEGIN);
  _free_lists[order]->order = _frame_array[0];
  _free_lists[order]->next = nullptr;
}


void* PageFrameAllocator::allocate(size_t requested_size) {
  if (!requested_size) {
    return nullptr;
  }

  // For each allocation request x, add the block size to x and
  // raise that value to a power of 2 s.t. x >= the original requested_size.
  requested_size = round_up_to_pow2(requested_size + sizeof(Block));
  int order = size_to_order(requested_size);

  if (order >= MAX_ORDER) {
    Kernel::panic("order >= MAX_ORDER\n");
  }

  printf("allocating physical memory of %d bytes...\n", requested_size);
  printf("searching free block from _free_lists[%d]...\n", order);

  // If there's an exact-fit free block from the free list,
  // then remove it from the free list and return that free block.
  Block* victim = _free_lists[order];

  if (victim && victim->order == size_to_order(requested_size)) {
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

  Block* block = reinterpret_cast<Block*>(p) - 1;  // 1 is for the header
  Block* buddy = get_buddy(block);

  printf("deallocating block 0x%x\n", block);

  // When you can’t find the buddy of the merged block or
  // the merged block size is maximum-block-size,
  // the allocator stops and put the merged block to the linked-list.
  while (!is_block_allocated(buddy) && block->order < MAX_ORDER - 1) {
    int block_idx = get_page_frame_index(block);
    int buddy_idx = get_page_frame_index(buddy);
    printf("merging blocks %d and %d\n", block_idx, buddy_idx);

    // Remove the buddy from the free list.
    free_list_del_entry(buddy);

    // Update _frame_array.
    mark_block_as_allocatable(block);

    if (block > buddy) {
      swap(block, buddy);
    }
    block->order++;

    buddy = get_buddy(block);
  }

  // Put back the merged block to the free list.
  free_list_add_head(block);

  // Update _frame_array.
  mark_block_as_allocatable(block);

  dump_memory_map();
}

void PageFrameAllocator::dump_memory_map() const {
  puts("--- dumping kernel heap ---");

  for (size_t i = 0; i < _frame_array_size; i++) {
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
      printf("[%d] -> ", ptr->order);
      ptr = ptr->next;
    }
    printf("[null]\n");
  }

  puts("--- end dumping kernel heap ---");
}

void* PageFrameAllocator::allocate_one_page_frame() {
  return allocate(PAGE_SIZE - sizeof(Block));
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
  if (_free_lists[block->order] == block) {
    return;
  }

  // If the list is empty
  if (!_free_lists[block->order]) {
    _free_lists[block->order] = block;
    block->next = nullptr;
  } else {
    block->next = _free_lists[block->order];
    _free_lists[block->order] = block;
  }
}

void PageFrameAllocator::free_list_del_entry(Block* block) {
  if (_free_lists[block->order] == block) {
    free_list_del_head(block);
    return;
  }

  Block* prev = nullptr;
  Block* ptr = _free_lists[block->order];

  while (ptr) {
    if (ptr == block) {
      prev->next = ptr->next;
      ptr->next = nullptr;
      break;
    }

    prev = ptr;
    ptr = ptr->next;
  }
}


PageFrameAllocator::Block* PageFrameAllocator::split_block(Block* block,
                                                           const int target_order) {
  if (!block) {
    Kernel::panic("kernel heap corrupted (block == nullptr)\n");
  }

  if (block->order < 0 || block->order > MAX_ORDER) {
    Kernel::panic("kernel heap corrupted (invalid block->order: %d)\n", block->order);
  }


  printf("---------------------------------\n");
  printf("comparing block order... %d vs %d\n", block->order, target_order);

  free_list_del_head(block);

  if (block->order == target_order) {
    return block;
  }

  // Update block headers
  block->order--;
  Pair<Block*, Block*> buddies = {block, get_buddy(block)};
  buddies.second->order = buddies.first->order;
  printf("buddies: b1 = 0x%x, b2 = 0x%x\n", buddies.first, buddies.second);

  // Add buddy1 and buddy2 to the free list.
  free_list_add_head(buddies.second);
  free_list_add_head(buddies.first);

  // Update _frame_array.
  mark_block_as_allocatable(buddies.first);
  mark_block_as_allocatable(buddies.second);

  // Continue splitting buddy1.
  return split_block(buddies.first, target_order);
}

PageFrameAllocator::Block* PageFrameAllocator::get_buddy(Block* block) {
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

bool PageFrameAllocator::is_block_allocated(const Block* block) {
  int idx = get_page_frame_index(block);
  return _frame_array[idx] == static_cast<int8_t>(ALLOCATED);
}

}  // namespace valkyrie::kernel

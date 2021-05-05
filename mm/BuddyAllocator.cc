// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/BuddyAllocator.h>

#include <Utility.h>
#include <dev/Console.h>
#include <kernel/Compiler.h>
#include <kernel/Kernel.h>
#include <libs/Math.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

BuddyAllocator::BuddyAllocator(const size_t zone_begin)
    : _zone_begin(zone_begin),
      _frame_array(),
      _free_lists() {
  const int order = size_to_order(get_zone_end() - _zone_begin);

  for (size_t i = 1; i < MAX_ORDER_NR_PAGES; i++) {
    _frame_array[i] = DONT_ALLOCATE;
  }
  _frame_array[0] = order;

  _free_lists[order] = reinterpret_cast<Block*>(_zone_begin);
  _free_lists[order]->order = _frame_array[0];
  _free_lists[order]->next = nullptr;
}


size_t BuddyAllocator::get_block_header_size() {
  return sizeof(Block);
}

void* BuddyAllocator::allocate(size_t requested_size) {
  if (unlikely(!requested_size)) {
    return nullptr;
  }

  // For each allocation request x, add the block header size to x and
  // raise that value to a power of 2 s.t. x >= the original requested_size.
  requested_size = normalize_size(requested_size + sizeof(Block));
  int order = size_to_order(requested_size);

  if (unlikely(order >= MAX_ORDER)) {
    printk("unable to allocate physical memory of %d bytes\n", requested_size);
    return nullptr;
  }

  // If there's an exact-fit free block from the free list,
  // then remove it from the free list and return that free block.
  Block* victim = nullptr;

  if (likely(victim = _free_lists[order])) {
    mark_block_as_allocated(victim);
    free_list_del_head(victim);
    goto out;
  }

  // Search larger free blocks.
  for (; order < MAX_ORDER; order++) {
    if (_free_lists[order]) {
      victim = _free_lists[order];
      break;
    }
  }

  if (unlikely(order >= MAX_ORDER)) {
    printk("unable to allocate physical memory of %d bytes\n", requested_size);
    return nullptr;
  }

  // Recursively divide the victim free block in half,
  // and update _free_lists until we've found an exact fit.
  victim = split_block(victim, size_to_order(requested_size));
  mark_block_as_allocated(victim);

out:
  return victim + 1;  // skip the header
}

void BuddyAllocator::deallocate(void* p) {
  if (unlikely(!p)) {
    return;
  }

  Block* block = reinterpret_cast<Block*>(p) - 1;  // 1 is for the header
  Block* buddy = get_buddy(block);

  // When you can’t find the buddy of the merged block or
  // the merged block size is maximum-block-size,
  // the allocator stops and put the merged block to the linked-list.
  while (!is_block_allocated(buddy) && block->order < MAX_ORDER - 1) {
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
}

void BuddyAllocator::dump_memory_map() const {
  puts("--- dumping buddy ---");

  for (size_t i = 0; i < MAX_ORDER_NR_PAGES; i++) {
    if (_frame_array[i] == DONT_ALLOCATE) {
      // Do nothing.
    } else if (_frame_array[i] == ALLOCATED) {
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
      printf("[%d 0x%x] -> ", ptr->order, ptr);
      ptr = ptr->next;
    }
    printf("(null)\n");
  }

  puts("--- end dumping buddy ---");
}

void* BuddyAllocator::allocate_one_page_frame() {
  return allocate(PAGE_SIZE - sizeof(Block));
}


int BuddyAllocator::get_page_frame_index(const Block* block) const {
  return (reinterpret_cast<size_t>(block) - _zone_begin) / PAGE_SIZE;
}

void BuddyAllocator::mark_block_as_allocated(const Block* block) {
  int idx = get_page_frame_index(block);
  int len = pow(2, block->order);
 
  for (int i = 0; i < len; i++) {
    _frame_array[idx + i] = ALLOCATED;
  }
}

void BuddyAllocator::mark_block_as_allocatable(const Block* block) {
  int idx = get_page_frame_index(block);
  int len = pow(2, block->order);

  _frame_array[idx] = block->order;
  for (int i = 1; i < len; i++) {
    _frame_array[idx + i] = DONT_ALLOCATE;
  }
}


void BuddyAllocator::free_list_del_head(Block* block) {
  if (unlikely(!block)) {
    Kernel::panic("kernel heap corrupted: free_list_del_head(nullptr)\n");
  }

  if (unlikely(block->order < 0 || block->order >= MAX_ORDER)) {
    Kernel::panic("kernel heap corrupted: block (0x%x) = {0x%x, 0x%x}\n",
                  block, block->next, block->order);
  }

  if (unlikely(!_free_lists[block->order])) {
    return;
  }

  _free_lists[block->order] = _free_lists[block->order]->next;
  block->next = nullptr;
}

void BuddyAllocator::free_list_add_head(Block* block) {
  if (unlikely(!block)) {
    Kernel::panic("kernel heap corrupted: free_list_add_head(nullptr)\n");
  }

  if (unlikely(block->order < 0 || block->order >= MAX_ORDER)) {
    Kernel::panic("kernel heap corrupted: block (0x%x) = {0x%x, 0x%x}\n",
                  block, block->next, block->order);
  }

  if (unlikely(_free_lists[block->order] == block)) {
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

void BuddyAllocator::free_list_del_entry(Block* block) {
  if (unlikely(!block)) {
    Kernel::panic("kernel heap corrupted: free_list_del_entry(nullptr)\n");
  }

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


BuddyAllocator::Block* BuddyAllocator::split_block(Block* block,
                                                           const int target_order) {
  if (unlikely(!block)) {
    Kernel::panic("kernel heap corrupted: block == nullptr\n");
  }

  if (unlikely(block->order < 0 || block->order >= MAX_ORDER)) {
    Kernel::panic("kernel heap corrupted: invalid block->order (%d)\n", block->order);
  }

  free_list_del_head(block);

  if (block->order == target_order) {
    return block;
  }

  // Update block headers
  block->order--;
  Pair<Block*, Block*> buddies = {block, get_buddy(block)};
  buddies.second->order = buddies.first->order;

  // Add buddy1 and buddy2 to the free list.
  free_list_add_head(buddies.second);
  free_list_add_head(buddies.first);

  // Update _frame_array.
  mark_block_as_allocatable(buddies.first);
  mark_block_as_allocatable(buddies.second);

  // Continue splitting buddy1.
  return split_block(buddies.first, target_order);
}

BuddyAllocator::Block* BuddyAllocator::get_buddy(Block* block) {
  const size_t b1 = reinterpret_cast<size_t>(block);
  const size_t b2 = b1 ^ (1 << (block->order)) * PAGE_SIZE;
  return reinterpret_cast<Block*>(b2);
}


int BuddyAllocator::size_to_order(const size_t size) const {
  // e.g., 4096 -> 0, 8192 -> 1, 16384 -> 2
  return log2(size / PAGE_SIZE);
}

int BuddyAllocator::order_to_size(const size_t order) const {
  return pow(2, order) * PAGE_SIZE;
}

bool BuddyAllocator::is_block_allocated(const Block* block) const {
  return _frame_array[get_page_frame_index(block)] == ALLOCATED;
}

size_t BuddyAllocator::normalize_size(size_t size) const {
  return round_up_to_pow_of_2(size);
}

size_t BuddyAllocator::get_zone_end() const {
  return _zone_begin + order_to_size(MAX_ORDER - 1);
}

}  // namespace valkyrie::kernel

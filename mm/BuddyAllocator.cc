// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/BuddyAllocator.h>

#include <Utility.h>
#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <libs/Math.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

BuddyAllocator::BuddyAllocator(const size_t zone_begin)
    : _zone_begin(zone_begin),
      _frame_array(),
      _free_lists(),
      _headers() {
  const int order = size_to_order(get_zone_end() - _zone_begin);

  // Initialize the frame array, as well as the header pool.
  _frame_array[0] = order;
  _headers[0].index = 0;

  for (size_t i = 1; i < MAX_ORDER_NR_PAGES; i++) {
    _frame_array[i] = DONT_ALLOCATE;
    _headers[i].index = i;
  }

  // Initialize the free lists.
  _free_lists[order] = &_headers[0];
  _free_lists[order]->order = _frame_array[0];
  _free_lists[order]->next = nullptr;
}


size_t BuddyAllocator::get_block_header_size() {
  return sizeof(BlockHeader);
}

void* BuddyAllocator::allocate(size_t requested_size) {
  if (!requested_size) [[unlikely]] {
    return nullptr;
  }

  // For each allocation request x, raise that value to
  // a power of 2 s.t. x >= the original requested_size.
  requested_size = normalize_size(requested_size);
  int order = size_to_order(requested_size);

  if (order >= MAX_ORDER) [[unlikely]] {
    printk("unable to allocate physical memory of %d bytes\n", requested_size);
    return nullptr;
  }

  // If there's an exact-fit free block from the free list,
  // then remove it from the free list and return that free block.
  BlockHeader* victim = nullptr;

  if ((victim = _free_lists[order])) [[likely]] {
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

  if (order >= MAX_ORDER || !victim) [[unlikely]] {
    printk("unable to allocate physical memory of %d bytes\n", requested_size);
    return nullptr;
  }

  // Recursively divide the victim free block in half,
  // and update _free_lists until we've found an exact fit.
  victim = split_block(victim, size_to_order(requested_size));
  mark_block_as_allocated(victim);

out:
  // Return the address of `victim->index`-th page frame.
  return get_page_frame(victim->index);
}

void BuddyAllocator::deallocate(void* p) {
  if (!p) [[unlikely]] {
    return;
  }

  BlockHeader* block = get_block_header(p);
  BlockHeader* buddy = get_buddy(block);

  // When you can’t find the buddy of the merged block or
  // the merged block size is maximum-block-size,
  // the allocator stops and put the merged block to the linked-list.
  while (block->order < MAX_ORDER - 1 && !is_block_allocated(buddy)) {
    // Remove the buddy from the free list.
    free_list_del_entry(buddy);

    // Update _frame_array.
    mark_block_as_allocatable(block);

    if (block > buddy) {
      swap(block, buddy);
    }
    block->order++;

    if (block->order < MAX_ORDER - 1) {
      buddy = get_buddy(block);
    }
  }

  // Put back the merged block to the free list.
  free_list_add_head(block);

  // Update _frame_array.
  mark_block_as_allocatable(block);
}

void BuddyAllocator::dump_memory_map() const {
  printf("--- dumping buddy ---\n");

  for (size_t i = 0; i < MAX_ORDER_NR_PAGES; i++) {
    if (_frame_array[i] == DONT_ALLOCATE) {
      // Do nothing.
    } else if (_frame_array[i] == ALLOCATED) {
      printf("<page frame #%d>: allocated page frame\n", i);
    } else {
      printf("<page frame #%d>: free page frame\n", i);
    }
  }

  printf("\n");

  for (int i = 0; i < MAX_ORDER; i++) {
    printf("_free_lists[%d]: ", i);
    BlockHeader* ptr = _free_lists[i];
    while (ptr) {
      printf("[%d 0x%x] -> ", ptr->order, ptr);
      ptr = ptr->next;
    }
    printf("(null)\n");
  }

  printf("--- end dumping buddy ---\n");
}

void* BuddyAllocator::allocate_one_page_frame() {
  return allocate(PAGE_SIZE);
}


BuddyAllocator::BlockHeader* BuddyAllocator::get_block_header(const void* p) {
  if (reinterpret_cast<size_t>(p) % PAGE_SIZE != 0) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: "
                  "get_block_header(0x%x) misaligned\n", p);
  }

  const int idx = (reinterpret_cast<size_t>(p) - _zone_begin) / PAGE_SIZE;

  if (idx < 0 || idx >= MAX_ORDER_NR_PAGES) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: "
                  "get_block_header(0x%x) idx out of bound: idx = %d\n",
                  p, idx);
  }

  return &_headers[idx];
}

void* BuddyAllocator::get_page_frame(const int index) const {
  return reinterpret_cast<void*>(_zone_begin + index * PAGE_SIZE);
}


void BuddyAllocator::mark_block_as_allocated(const BlockHeader* block) {
  int idx = block->index;
  int len = pow(2, block->order);
 
  for (int i = 0; i < len; i++) {
    _frame_array[idx + i] = ALLOCATED;
  }
}

void BuddyAllocator::mark_block_as_allocatable(const BlockHeader* block) {
  int idx = block->index;
  int len = pow(2, block->order);

  _frame_array[idx] = block->order;
  for (int i = 1; i < len; i++) {
    _frame_array[idx + i] = DONT_ALLOCATE;
  }
}


void BuddyAllocator::free_list_del_head(BlockHeader* block) {
  if (!block) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: free_list_del_head(nullptr)\n");
  }

  if (block->order < 0 || block->order >= MAX_ORDER) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: block (0x%x) = {0x%x, 0x%x}\n",
                  block, block->next, block->order);
  }

  if (!_free_lists[block->order]) [[unlikely]] {
    return;
  }

  _free_lists[block->order] = _free_lists[block->order]->next;
  block->next = nullptr;
}

void BuddyAllocator::free_list_add_head(BlockHeader* block) {
  if (!block) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: free_list_add_head(nullptr)\n");
  }

  if (block->order < 0 || block->order >= MAX_ORDER) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: block (0x%x) = {0x%x, 0x%x}\n",
                  block, block->next, block->order);
  }

  if (_free_lists[block->order] == block) [[unlikely]] {
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

void BuddyAllocator::free_list_del_entry(BlockHeader* block) {
  if (!block) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: free_list_del_entry(nullptr)\n");
  }

  if (_free_lists[block->order] == block) {
    free_list_del_head(block);
    return;
  }

  BlockHeader* prev = nullptr;
  BlockHeader* ptr = _free_lists[block->order];

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


BuddyAllocator::BlockHeader* BuddyAllocator::split_block(BlockHeader* block,
                                                         const int target_order) {
  if (!block) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: block == nullptr\n");
  }

  if (block->order < 0 || block->order >= MAX_ORDER) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: invalid block->order (%d)\n", block->order);
  }

  free_list_del_head(block);

  if (block->order == target_order) {
    return block;
  }

  // Update block headers
  block->order--;
  Pair<BlockHeader*, BlockHeader*> buddies = {block, get_buddy(block)};
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

BuddyAllocator::BlockHeader* BuddyAllocator::get_buddy(BlockHeader* block) {
  const size_t b1 = reinterpret_cast<size_t>(get_page_frame(block->index));
  const size_t b2 = b1 ^ (1 << (block->order)) * PAGE_SIZE;
  if (b2 < _zone_begin || b2 >= get_zone_end()) {
    Kernel::panic("b1 = 0x%x { .order = %d, .index = %d }, b2 = 0x%x\n", b1, block->order, block->index, b2);
  }
  return get_block_header(reinterpret_cast<void*>(b2));
}


int BuddyAllocator::size_to_order(const size_t size) const {
  // e.g., 4096 -> 0, 8192 -> 1, 16384 -> 2
  return log2(size / PAGE_SIZE);
}

int BuddyAllocator::order_to_size(const size_t order) const {
  return pow(2, order) * PAGE_SIZE;
}

bool BuddyAllocator::is_block_allocated(const BlockHeader* block) const {
  return _frame_array[block->index] == ALLOCATED;
}

size_t BuddyAllocator::normalize_size(size_t size) const {
  return round_up_to_pow_of_2(size);
}

size_t BuddyAllocator::get_zone_end() const {
  return _zone_begin + order_to_size(MAX_ORDER - 1);
}

}  // namespace valkyrie::kernel

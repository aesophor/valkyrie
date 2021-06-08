// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_BUDDY_ALLOCATOR_H_
#define VALKYRIE_BUDDY_ALLOCATOR_H_

#include <Types.h>

#define MAX_ORDER          10
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

#define ALLOCATED          static_cast<int8_t>(-1)
#define DONT_ALLOCATE      static_cast<int8_t>(-2)

namespace valkyrie::kernel {

class BuddyAllocator {
 public:
  explicit BuddyAllocator(const size_t zone_begin);

  ~BuddyAllocator() = default;
  BuddyAllocator(const BuddyAllocator&) = delete;
  BuddyAllocator(BuddyAllocator&&) = delete;
  BuddyAllocator& operator =(const BuddyAllocator&) = delete;
  BuddyAllocator& operator =(BuddyAllocator&&) = delete;

  static size_t get_block_header_size();

  void* allocate(size_t requested_size);
  void  deallocate(void* p);
  void  dump_memory_map() const;
  void* allocate_one_page_frame();

 private:
  struct BlockHeader {
    BlockHeader* next;
    int32_t order;
    int32_t index;
  };

  BlockHeader* get_block_header(const void* p);
  void* get_page_frame(const int index) const;
  
  void mark_block_as_allocated(const BlockHeader* block);
  void mark_block_as_allocatable(const BlockHeader* block);

  void free_list_del_head(BlockHeader* block);
  void free_list_add_head(BlockHeader* block);
  void free_list_del_entry(BlockHeader* block);

  // Recursively split the given block
  // until it is exactly the size of PAGE_SIZE * 2^`target_order`.
  BlockHeader* split_block(BlockHeader* block, const int target_order);
  BlockHeader* get_buddy(BlockHeader* block);

  int size_to_order(const size_t size) const;
  int order_to_size(const size_t order) const;
  bool is_block_allocated(const BlockHeader* block) const;
  size_t normalize_size(size_t size) const;
  size_t get_zone_end() const;



  // The address of the beginning of this zone.
  // Each buddy allocator manages a "zone".
  const size_t _zone_begin;

  // The Frame Array (or "The Array")
  // See: https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab3.html#data-structure
  //
  // Each element represents exactly one `PAGE_SIZE` physical page frame.
  // For example, if `PAGE_SIZE` is 4096, then each element represents
  // a 4KB physical page frame.
  //
  // Possible values of each element x ∈ _frame_array:
  // (1) x >= 0 : 2^x contiguous frames available starting at this block.
  // (2) x == ALLOCATED : this block is allocated.
  // (3) x == DONT_ALLOCATE : this block is free,
  //                          but it belongs to another larger contiguous block,
  //                          so the buddy allocator shouldn't directly allocate it.
  int8_t _frame_array[MAX_ORDER_NR_PAGES];

  // An array of Singly-linked Lists of free page frames of different sizes.
  // See: https://www.kernel.org/doc/gorman/html/understand/understand009.html
  BlockHeader* _free_lists[MAX_ORDER];

  // Header pool
  BlockHeader _headers[MAX_ORDER_NR_PAGES];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_BUDDY_ALLOCATOR_H_

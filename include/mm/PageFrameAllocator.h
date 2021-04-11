// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PAGE_FRAME_ALLOCATOR_H_
#define VALKYRIE_PAGE_FRAME_ALLOCATOR_H_

#define PAGE_SIZE  4096
#define HEAP_BEGIN 0X10000000
#define HEAP_END   0x10200000

#define MAX_ORDER 10

#define ALLOCATED     static_cast<int8_t>(-1)
#define DONT_ALLOCATE static_cast<int8_t>(-2)

#include <Types.h>

namespace valkyrie::kernel {

class PageFrameAllocator {
 public:
  PageFrameAllocator();
  ~PageFrameAllocator() = default;

  void* allocate(size_t requested_size);
  void  deallocate(void* p);
  void  dump_memory_map() const;

  void* allocate_one_page_frame();

  static size_t get_block_header_size();

 private:
  struct Block {
    Block* next;
    int64_t order;
  };

  int  get_page_frame_index(const Block* block) const;
  void mark_block_as_allocated(const Block* block);
  void mark_block_as_allocatable(const Block* block);

  void free_list_del_head(Block* block);
  void free_list_add_head(Block* block);
  void free_list_del_entry(Block* block);

  int size_to_order(const size_t size);

  bool is_block_allocated(const Block* block);

  // Recursively split the given block
  // until it is exactly the size of PAGE_SIZE * 2^`target_order`.
  Block* split_block(Block* block, const int target_order);
  Block* get_buddy(Block* block);

  size_t normalize_size(size_t size);
  size_t round_up_to_pow_of_2(size_t x);

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
  int8_t _frame_array[(HEAP_END - HEAP_BEGIN) / PAGE_SIZE];
  const size_t _frame_array_size;

  // An array of Singly-linked Lists of free page frames of different sizes.
  // See: https://www.kernel.org/doc/gorman/html/understand/understand009.html
  Block* _free_lists[MAX_ORDER];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PAGE_FRAME_ALLOCATOR_H_

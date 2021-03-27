// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PAGE_FRAME_ALLOCATOR_H_
#define VALKYRIE_PAGE_FRAME_ALLOCATOR_H_

#define PAGE_SIZE  4096
#define HEAP_BEGIN 0X10000000
#define HEAP_END   0x10001000

#define MAX_ORDER 10

#define ALLOCATED     -1
#define DONT_ALLOCATE -2

#include <Types.h>

namespace valkyrie::kernel {

class PageFrameAllocator {
 public:
  PageFrameAllocator();
  ~PageFrameAllocator() = default;

  void* allocate(size_t requested_size);
  void  deallocate(void* p);

 private:
  struct PageFrameHeader {
    size_t size;  // exponent
    PageFrameHeader* next;
  };

  void* get_page_frame_by_index(const size_t index) const;
  int   get_page_frame_index(PageFrameHeader* header) const;
  void  mark_page_frame_as_allocated(PageFrameHeader* header);
  void  dump_memory_map() const;

  static size_t get_free_list_index(const size_t x);

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
  size_t _frame_array[(HEAP_END - HEAP_BEGIN) / PAGE_SIZE];
  const size_t _frame_array_size;


  // An array of Singly-linked Lists of free page frames of different sizes.
  // See: https://www.kernel.org/doc/gorman/html/understand/understand009.html
  PageFrameHeader* _free_lists[MAX_ORDER];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PAGE_FRAME_ALLOCATOR_H_
// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MEMORY_MANAGER_H_
#define VALKYRIE_MEMORY_MANAGER_H_

#include <PageFrameAllocator.h>

namespace valkyrie::kernel {

class MemoryManager {
 public:
  static MemoryManager* get_instance();
  ~MemoryManager() = default;

  void* kmalloc(size_t size);
  void  kfree(void* p);

 private:
  MemoryManager();

  void* allocate_page_frame();
  void  deallocate_page_frame(void* p);

  PageFrameAllocator _page_frame_allocator;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MEMORY_MANAGER_H_

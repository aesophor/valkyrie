// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MEMORY_MANAGER_H_
#define VALKYRIE_MEMORY_MANAGER_H_

#include <PageFrameAllocator.h>
#include <SlobAllocator.h>

namespace valkyrie::kernel {

class MemoryManager {
 public:
  static MemoryManager* get_instance();
  ~MemoryManager() = default;

  void* kmalloc(size_t size);
  void  kfree(void* p);
  void  dump_physical_memory_map() const;

 private:
  MemoryManager();

  PageFrameAllocator _page_frame_allocator;
  SlobAllocator _slob_allocator;
};

}  // namespace valkyrie::kernel


extern "C" void* kmalloc(const size_t requested_size);
extern "C" void  kfree(void* p);

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* p) noexcept;
void operator delete[](void* p) noexcept;

void operator delete(void* p, size_t) noexcept;
void operator delete[](void* p, size_t) noexcept;

#endif  // VALKYRIE_MEMORY_MANAGER_H_

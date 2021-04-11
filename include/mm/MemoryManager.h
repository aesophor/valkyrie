// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MEMORY_MANAGER_H_
#define VALKYRIE_MEMORY_MANAGER_H_

#include <mm/PageFrameAllocator.h>
#include <mm/SlobAllocator.h>
#include <mm/AddressSanitizer.h>

namespace valkyrie::kernel {

class MemoryManager {
 public:
  static MemoryManager& get_instance();
  ~MemoryManager() = default;

  void* kmalloc(size_t size);
  void  kfree(void* p);

  void dump_page_frame_allocator_info() const;
  void dump_slob_allocator_info() const;

 private:
  MemoryManager();

  PageFrameAllocator _page_frame_allocator;
  SlobAllocator _slob_allocator;
  AddressSanitizer _asan;
};

}  // namespace valkyrie::kernel


extern "C" inline void* kmalloc(const size_t requested_size) {
  return valkyrie::kernel::MemoryManager::get_instance().kmalloc(requested_size);
}

extern "C" inline void kfree(void* p) {
  valkyrie::kernel::MemoryManager::get_instance().kfree(p);
}

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* p) noexcept;
void operator delete[](void* p) noexcept;

void operator delete(void* p, size_t) noexcept;
void operator delete[](void* p, size_t) noexcept;

#endif  // VALKYRIE_MEMORY_MANAGER_H_

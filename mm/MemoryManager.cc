// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <MemoryManager.h>

namespace valkyrie::kernel {

MemoryManager* MemoryManager::get_instance() {
  static MemoryManager instance;
  return &instance;
}

MemoryManager::MemoryManager()
    : _page_frame_allocator() {}


void* MemoryManager::kmalloc(size_t size) {
  return _page_frame_allocator.allocate(size);
}

void MemoryManager::kfree(void* p) {
  _page_frame_allocator.deallocate(p);
}

void MemoryManager::dump_physical_memory_map() const {
  _page_frame_allocator.dump_memory_map();
}


void* MemoryManager::allocate_page_frame() {

}

void MemoryManager::deallocate_page_frame(void* p) {

}

}  // namespace valkyrie::kernel

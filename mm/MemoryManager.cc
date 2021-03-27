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
  if (!p) {
    return;
  }
}


void* MemoryManager::allocate_page_frame() {

}

void MemoryManager::deallocate_page_frame(void* p) {

}

}  // namespace valkyrie::kernel
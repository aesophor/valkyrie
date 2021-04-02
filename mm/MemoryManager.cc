// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/MemoryManager.h>

namespace valkyrie::kernel {

MemoryManager* MemoryManager::get_instance() {
  static MemoryManager instance;
  return &instance;
}

MemoryManager::MemoryManager()
    : _page_frame_allocator(),
      _slob_allocator(&_page_frame_allocator) {}


void* MemoryManager::kmalloc(size_t size) {
  // FIXME: clean up this shit ... ( ´•̥̥̥ω•̥̥̥` ) 
  if (size +
      PageFrameAllocator::get_block_header_size() +
      SlobAllocator::get_chunk_header_size() >= PAGE_SIZE) {
    return _page_frame_allocator.allocate(size);
  } else {
    return _slob_allocator.allocate(size);
  }
}

void MemoryManager::kfree(void* p) {
  size_t addr = reinterpret_cast<size_t>(p) -
                PageFrameAllocator::get_block_header_size();

  if (addr % PAGE_SIZE == 0) {
    _page_frame_allocator.deallocate(p);
  } else {
    _slob_allocator.deallocate(p);
  }
}


void MemoryManager::dump_page_frame_allocator_info() const {
  _page_frame_allocator.dump_memory_map();
}

void MemoryManager::dump_slob_allocator_info() const {
  _slob_allocator.dump_slob_info();
}

}  // namespace valkyrie::kernel


void* operator new(size_t size) {
  return kmalloc(size);
}

void* operator new[](size_t size) {
  return kmalloc(size);
}

void operator delete(void* p) noexcept {
  return kfree(p);
}

void operator delete[](void* p) noexcept {
  return kfree(p);
}

void operator delete(void* p, size_t) noexcept {
  return kfree(p);
}

void operator delete[](void* p, size_t) noexcept {
  return kfree(p);
}

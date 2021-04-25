// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/MemoryManager.h>

#include <dev/Mailbox.h>

namespace valkyrie::kernel {

MemoryManager& MemoryManager::get_instance() {
  static MemoryManager instance;
  return instance;
}

MemoryManager::MemoryManager()
    : _ram_size(Mailbox::get_instance().get_arm_memory().second),
      _buddy_allocator(),
      _slob_allocator(&_buddy_allocator),
      _asan() {}


void* MemoryManager::get_free_page() {
  return _buddy_allocator.allocate_one_page_frame();
}

void* MemoryManager::kmalloc(size_t size) {
  // FIXME: clean up this shit ... ( ´•̥̥̥ω•̥̥̥` ) 
  if (size +
      BuddyAllocator::get_block_header_size() +
      SlobAllocator::get_chunk_header_size() >= PAGE_SIZE) {
    return _buddy_allocator.allocate(size);
  } else {
    auto ret = _slob_allocator.allocate(size);
    _asan.mark_allocated(ret);
    return ret;
  }
}

void MemoryManager::kfree(void* p) {
  size_t addr = reinterpret_cast<size_t>(p) -
                BuddyAllocator::get_block_header_size();

  if (addr % PAGE_SIZE == 0) {
    _buddy_allocator.deallocate(p);
  } else {
    _asan.mark_free_chk(p);
    _slob_allocator.deallocate(p);
  }
}


void MemoryManager::dump_page_frame_allocator_info() const {
  _buddy_allocator.dump_memory_map();
}

void MemoryManager::dump_slob_allocator_info() const {
  _slob_allocator.dump_slob_info();
}


size_t MemoryManager::get_ram_size() const {
  return _ram_size;
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

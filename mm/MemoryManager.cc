// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/MemoryManager.h>

#include <dev/Console.h>
#include <driver/Mailbox.h>
#include <mm/Page.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

MemoryManager& MemoryManager::get_instance() {
  static MemoryManager instance;
  return instance;
}

MemoryManager::MemoryManager()
    : _ram_size(Mailbox::get_instance().get_arm_memory().second),
      _zones{Zone(0x10000000), Zone(0x10200000)},
      _kasan() {}


void* MemoryManager::get_free_page() {
  void* ret = _zones[0].buddy_allocator.allocate_one_page_frame();
  _kasan.mark_allocated(ret);
  return ret;
}

void* MemoryManager::kmalloc(size_t size) {
  void* ret;

  if (size + SlobAllocator::get_chunk_header_size() >= PAGE_SIZE) {
    ret = _zones[0].buddy_allocator.allocate(size);
  } else {
    ret = _zones[1].slob_allocator.allocate(size);
  }

  _kasan.mark_allocated(ret);
  return ret;
}

void MemoryManager::kfree(void* p) {
  _kasan.mark_free_chk(p);

  if (reinterpret_cast<size_t>(p) % PAGE_SIZE == 0) {
    _zones[0].buddy_allocator.deallocate(p);
  } else {
    _zones[1].slob_allocator.deallocate(p);
  }
}


void MemoryManager::dump_page_frame_allocator_info() const {
  _zones[0].buddy_allocator.dump_memory_map();
}

void MemoryManager::dump_slob_allocator_info() const {
  _zones[0].slob_allocator.dump_slob_info();
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

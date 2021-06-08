// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/MemoryManager.h>

#include <dev/Console.h>
#include <driver/Mailbox.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

MemoryManager& MemoryManager::get_instance() {
  static MemoryManager instance;
  return instance;
}

MemoryManager::MemoryManager()
    : _ram_size(Mailbox::get_instance().get_arm_memory().second),
      _zones{Zone(0x10000000), Zone(0x10200000)},
      _kasan() {}


Zone* MemoryManager::initialize_zones() {
  printk("sizeof(BuddyAllocator) = %d\n", sizeof(BuddyAllocator));
  printk("sizeof(SlobAllocator) = %d\n", sizeof(SlobAllocator));

  size_t nr_pages = (_ram_size - 0x10000000) / PAGE_SIZE;
  printk("%d of pages to manage\n", nr_pages);

  size_t nr_zones = nr_pages / Zone::get_pages_count();
  printk("%d zones are needed\n", nr_zones);

  printk("zone structs size = 0x%x\n", nr_zones * (sizeof(BuddyAllocator) + sizeof(SlobAllocator)));

  /*
  Zone* zone = reinterpret_cast<Zone*>(0x400000);
  for (size_t i = 0; i < nr_zones; i++, zone++) {
    zone->buddy_allocator = BuddyAllocator(reinterpret_cast<size_t>(zone));
    zone->slob_allocator = SlobAllocator(&zone->buddy_allocator);
  }
  */

  while (1);
  return reinterpret_cast<Zone*>(0x400000);
}

void* MemoryManager::get_free_page() {
  return _zones[0].buddy_allocator.allocate_one_page_frame();
}

void* MemoryManager::kmalloc(size_t size) {
  if (size + SlobAllocator::get_chunk_header_size() >= PAGE_SIZE) {
    return _zones[0].buddy_allocator.allocate(size);
  } else {
    auto ret = _zones[1].slob_allocator.allocate(size);
    _kasan.mark_allocated(ret);
    return ret;
  }
}

void MemoryManager::kfree(void* p) {
  if (is_address_aligned(p)) {
    _zones[0].buddy_allocator.deallocate(p);
  } else {
    _kasan.mark_free_chk(p);
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

bool MemoryManager::is_address_aligned(const void* p) const {
  return reinterpret_cast<size_t>(p) % PAGE_SIZE == 0;
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

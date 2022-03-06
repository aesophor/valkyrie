// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/MemoryManager.h>

#include <dev/Console.h>
#include <driver/Mailbox.h>
#include <kernel/Kernel.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

MemoryManager::MemoryManager()
    : _ram_size(Mailbox::the().get_arm_memory().second),
      _zones{Zone(0x10000000), Zone(0x10200000)},
      _ref_counts(),
      _page_writable(),
      _kasan() {}

void *MemoryManager::get_free_page(bool physical) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  void *ret = _zones[0].buddy_allocator.allocate_one_page_frame();
  _kasan.mark_allocated(ret);

  return physical ? ret
                  : reinterpret_cast<void *>(KERNEL_VA_BASE + reinterpret_cast<size_t>(ret));
}

void *MemoryManager::kmalloc(size_t size) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  void *ret;

  if (size + SlobAllocator::get_chunk_header_size() >= PAGE_SIZE) {
    ret = _zones[0].buddy_allocator.allocate(size);
  } else {
    ret = _zones[1].slob_allocator.allocate(size);
  }

  _kasan.mark_allocated(ret);
  return ret;
}

void MemoryManager::kfree(void *p) {
  // If `p` is a virtual kernel address, convert it to physical.
  size_t addr = reinterpret_cast<size_t>(p);
  if (addr >= KERNEL_VA_BASE) {
    addr -= KERNEL_VA_BASE;
  }
  p = reinterpret_cast<void *>(addr);

  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  _kasan.mark_free_chk(p);

  if (reinterpret_cast<size_t>(p) % PAGE_SIZE == 0) {
    _zones[0].buddy_allocator.deallocate(p);
  } else {
    _zones[1].slob_allocator.deallocate(p);
  }
}

String MemoryManager::get_buddy_info() const {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  return _zones[0].buddy_allocator.to_string();
}

String MemoryManager::get_slob_info() const {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  return _zones[1].slob_allocator.to_string();
}

void MemoryManager::dump_buddy_allocator_info() const {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  _zones[0].buddy_allocator.dump();
}

void MemoryManager::dump_slob_allocator_info() const {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  _zones[1].slob_allocator.dump();
}

void MemoryManager::dump_kasan_info() const {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  _kasan.show();
}

size_t MemoryManager::get_ram_size() const {
  return _ram_size;
}

int MemoryManager::inc_page_ref_count(const void *p_addr) {
  size_t idx = get_page_ref_idx(p_addr);

  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  return ++_ref_counts[idx];
}

int MemoryManager::dec_page_ref_count(const void *p_addr) {
  size_t idx = get_page_ref_idx(p_addr);

  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  return --_ref_counts[idx];
}

int MemoryManager::get_page_ref_count(const void *p_addr) const {
  size_t idx = get_page_ref_idx(p_addr);

  const LockGuard<RecursiveMutex> lock(Kernel::mutex);
  return _ref_counts[idx];
}

int MemoryManager::get_page_ref_idx(const void *p_addr) const {
  size_t addr = reinterpret_cast<size_t>(p_addr);

  if (addr < _zones[0].begin_addr || addr >= _zones[1].begin_addr) [[unlikely]] {
    Kernel::panic("get_page_ref_idx(): p_addr (0x%p) out of bound\n", p_addr);
  }

  if (addr % PAGE_SIZE) [[unlikely]] {
    Kernel::panic("get_page_ref_idx(): p_addr (0x%p) is not a valid page address\n", p_addr);
  }

  return (addr - _zones[0].begin_addr) >> 12;
}

}  // namespace valkyrie::kernel

void *operator new(size_t size) {
  return kmalloc(size);
}

void *operator new[](size_t size) {
  return kmalloc(size);
}

void operator delete(void *p) noexcept {
  return kfree(p);
}

void operator delete[](void *p) noexcept {
  return kfree(p);
}

void operator delete(void *p, size_t) noexcept {
  return kfree(p);
}

void operator delete[](void *p, size_t) noexcept {
  return kfree(p);
}

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MEMORY_MANAGER_H_
#define VALKYRIE_MEMORY_MANAGER_H_

#include <Mutex.h>
#include <Singleton.h>
#include <String.h>

#include <mm/AddressSanitizer.h>
#include <mm/Zone.h>
#include <mm/mmu.h>

namespace valkyrie::kernel {

class MemoryManager : public Singleton<MemoryManager> {
 public:
  void *get_free_page(bool physical = false);

  // Operates on virtual memory addresses.
  void *kmalloc(size_t size);
  void kfree(void *p);

  String get_buddy_info() const;
  String get_slob_info() const;

  void dump_buddy_allocator_info() const;
  void dump_slob_allocator_info() const;
  void dump_kasan_info() const;

  size_t get_ram_size() const;

  int inc_page_ref_count(const void *p_addr);
  int dec_page_ref_count(const void *p_addr);
  int get_page_ref_count(const void *p_addr) const;

 protected:
  MemoryManager();

 private:
  int get_page_ref_idx(const void *p_addr) const;

  const size_t _ram_size;
  Zone _zones[2];

  // XXX: Copy on write, refactor this
  int _ref_counts[MAX_ORDER_NR_PAGES];
  bool _page_writable[MAX_ORDER_NR_PAGES];

  AddressSanitizer _kasan;
};

}  // namespace valkyrie::kernel

extern "C" void *switch_user_va_space(void *ttbr0_el1);

extern "C" inline void *get_free_page(bool physical = false) {
  return valkyrie::kernel::MemoryManager::the().get_free_page(physical);
}

extern "C" inline void *kmalloc(const size_t requested_size) {
  return valkyrie::kernel::MemoryManager::the().kmalloc(requested_size);
}

extern "C" inline void kfree(void *p) {
  valkyrie::kernel::MemoryManager::the().kfree(p);
}

void *operator new(size_t size);
void *operator new[](size_t size);

void operator delete(void *p) noexcept;
void operator delete[](void *p) noexcept;

void operator delete(void *p, size_t) noexcept;
void operator delete[](void *p, size_t) noexcept;

#endif  // VALKYRIE_MEMORY_MANAGER_H_

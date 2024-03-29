// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MEMORY_MANAGER_H_
#define VALKYRIE_MEMORY_MANAGER_H_

#include <Singleton.h>
#include <String.h>
#include <mm/AddressSanitizer.h>
#include <mm/Zone.h>

namespace valkyrie::kernel {

class MemoryManager : public Singleton<MemoryManager> {
 public:
  void* get_free_page();
  void* kmalloc(size_t size);
  void  kfree(void* p);

  String get_buddy_info() const;
  String get_slob_info() const;

  void dump_buddy_allocator_info() const;
  void dump_slob_allocator_info() const;
  void dump_kasan_info() const;

  size_t get_ram_size() const;

 protected:
  MemoryManager();

 private:
  const size_t _ram_size;
  Zone _zones[2];
  AddressSanitizer _kasan;
};

}  // namespace valkyrie::kernel


extern "C" void* switch_user_va_space(void* ttbr0_el1);

extern "C" inline void* get_free_page() {
  return valkyrie::kernel::MemoryManager::get_instance().get_free_page();
}

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

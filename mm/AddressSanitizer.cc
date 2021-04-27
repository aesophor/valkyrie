// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/AddressSanitizer.h>

#include <kernel/Kernel.h>

namespace valkyrie::kernel {

AddressSanitizer::AddressSanitizer() : _allocated_pointers() {}


bool AddressSanitizer::mark_free_chk(void *p) {
  for (int i = 0; i < 1000; i++) {
    if (p == _allocated_pointers[i]) {
      _allocated_pointers[i] = nullptr;
      //show();
      return true;
    }
  }

  Kernel::panic("*** kasan: double free detected at 0x%x ***\n", p);
  return false;
}

void AddressSanitizer::mark_allocated(void* p) {
  for (int i = 0; i < 1000; i++) {
    if (_allocated_pointers[i] == p) {
      Kernel::panic("*** kasan: pointer 0x%x is already allocated...\n", p);
    }
  }

  for (int i = 0; i < 1000; i++) {
    if (!_allocated_pointers[i]) {
      _allocated_pointers[i] = p;
      //show();
      return;
    }
  }
}

void AddressSanitizer::show() {
  printk("allocated ptrs: [");
  for (int i = 0; i < 1000; i++) {
    if (_allocated_pointers[i]) {
      printf("0x%x,", reinterpret_cast<size_t>(_allocated_pointers[i]));
    }
  }
  printf("]\n");
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_IO_H_
#define VALKYRIE_IO_H_

#include <Types.h>

#define KERNEL_VA_SPACE_BASE (0xffff000000000000)
#define MMIO_BASE            (KERNEL_VA_SPACE_BASE + 0x3f000000)

namespace valkyrie::kernel::io {

template <typename T>
T get(const size_t addr) {
  return *reinterpret_cast<const T*>(addr);
}

template <typename T>
void put(const size_t addr, const T data) {
  *reinterpret_cast<T*>(addr) = data;
}

inline void delay(size_t cycles) {
  while (cycles--) {
    asm volatile("nop");
  }
}

}  // namespace valkyrie::kernel::io

#endif  // VALKYRIE_IO_H_

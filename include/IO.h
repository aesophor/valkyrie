// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_IO_H_
#define VALKYRIE_IO_H_

#include <Types.h>

namespace valkyrie::kernel::io {

template <typename T>
T read(const size_t src_addr) {
  return *reinterpret_cast<const T*>(src_addr);
}

template <typename T>
void write(const size_t dest_addr, const T data) {
  *reinterpret_cast<T*>(dest_addr) = data;
}


inline void delay(size_t cycles) {
  while (cycles--) {
    asm volatile("nop");
  }
}

}  // namespace valkyrie::kernel::io

#endif  // VALKYRIE_MINI_UART_H_

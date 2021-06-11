// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

extern "C" [[noreturn]] void _start() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp] \n\
                bl main          \n\
                b  exit            ");
}

extern "C" void __libc_putchar(void*, const char c) {
  write(1, &c, 1);
}

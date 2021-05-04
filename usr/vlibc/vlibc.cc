// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

void __libc_putchar(void*, const char c) {
  uart_putchar(c);
}

[[noreturn]] void __libc_start_main() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp] \n\
                bl main          \n\
                b  exit            ");
}

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Console.h>

namespace valkyrie::kernel {

void console_init(MiniUART* mini_uart) {
  ::mini_uart = mini_uart;
}

void gets(char* s) {
  ::mini_uart->gets(s);
}

void puts(const char* s) {
  ::mini_uart->puts(s);
}

void putchar(const char c) {
  ::mini_uart->putchar(c);
}

void print_hex(const uint32_t value) {
  static const char* hex = "0123456789abcdef";
  char* addr = "0x00000000";

  size_t offset;
  for (size_t i = 2, offset = 28; i < 2 + 8; i++, offset -= 4) {
    addr[i] = hex[(value >> offset) & 0x0f];
  }

  puts(addr);
}

}  // namespace valkyrie::kernel

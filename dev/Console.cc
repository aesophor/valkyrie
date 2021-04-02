// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/Console.h>

namespace {

valkyrie::kernel::MiniUART* mini_uart;

// Required by tfp_printf()
void _putchar(void*, char c) {
  ::mini_uart->putchar(c);
}

}  // namespace


namespace valkyrie::kernel::console {

void initialize(MiniUART* mini_uart) {
  ::mini_uart = mini_uart;
  init_printf(nullptr, _putchar);
}

void set_color(Color fg_color, bool bold) {
  printf("\033[%d;3%dm", bold, fg_color);
}

void clear_color() {
  puts("\033[0m", /*newline=*/false);
}

}  // namespace valkyrie::kernel::console


extern "C" {

char _recv() {
  return ::mini_uart->recv();
}

char getchar() {
  return ::mini_uart->getchar();
}

void putchar(const char c) {
  ::mini_uart->putchar(c);
}

void gets(char* s) {
  ::mini_uart->gets(s);
}

void puts(const char* s, bool newline) {
  ::mini_uart->puts(s, newline);
}

}

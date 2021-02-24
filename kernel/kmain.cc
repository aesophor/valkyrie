// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mini_uart.h>

namespace {

// Initialize .bss section to 0
void init_bss(char* begin, char* end) {
  while (begin <= end) {
    *begin++ = 0x00;
  }
}

}  // namespace


extern "C" void kmain(char* bss_start, char* bss_end) {
  init_bss(bss_start, bss_end);

  valkyrie::mini_uart uart;
  uart.write('O');
  uart.write('M');
  uart.write('G');
  uart.write('!');

  while (1);
}

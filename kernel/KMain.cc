// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <MiniUART.h>
#include <String.h>

using namespace valkyrie::kernel;

namespace {

// Initialize .bss section to 0
void init_bss(char* begin, char* end) {
  while (begin <= end) {
    *begin++ = 0x00;
  }
}

}  // namespace


extern "C" void kmain(char* bss_start, char* bss_end) {
  ::init_bss(bss_start, bss_end);

  MiniUART uart;

  uart.puts("Valkyrie Operating System");
  uart.puts("=========================");

  char c = uart.getchar();
  uart.putchar(c);

  uart.puts("bye...");

  while (1);
}

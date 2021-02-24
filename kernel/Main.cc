// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <MiniUART.h>
#include <String.h>

using namespace valkyrie::kernel;

extern "C" void kmain(char* bss_start, char* bss_end) {
  // Initialize bss segment to 0
  memset(bss_start, 0, bss_end - bss_start);

  MiniUART uart;
  uart.puts("Valkyrie Operating System");
  uart.puts("=========================");

  char buf[256];

  while (true) {
    memset(buf, 0, sizeof(buf));
    uart.gets(buf);
    uart.puts(buf);
  }

  uart.puts("bye...");
  while (1);
}

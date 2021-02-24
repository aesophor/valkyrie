// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <String.h>

namespace valkyrie::kernel {

Kernel::Kernel() : _mini_uart() {}


void Kernel::run() {
  _mini_uart.puts("Valkyrie Operating System");
  _mini_uart.puts("=========================");

  char buf[256];

  while (true) {
    memset(buf, 0, sizeof(buf));
    _mini_uart.gets(buf);
    _mini_uart.puts(buf);
  }

  _mini_uart.puts("bye...");
  while (1);
}


extern "C" void kmain(char* bss_start, char* bss_end) {
  // Initialize bss segment to 0
  valkyrie::kernel::memset(bss_start, 0, bss_end - bss_start);
  
  // Run the kernel
  valkyrie::kernel::Kernel kernel;
  kernel.run();
}

}  // namespace valkyrie::kernel

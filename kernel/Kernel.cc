// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <String.h>

namespace valkyrie::kernel {

Kernel::Kernel() : _mini_uart() {}


void Kernel::run() {
  puts("Valkyrie Operating System");
  puts("=========================");

  char buf[256];

  while (true) {
    memset(buf, 0, sizeof(buf));
    gets(buf);
    puts(buf);
  }

  puts("bye...");

  while (1);
}


void Kernel::gets(char* s) {
  _mini_uart.gets(s);
}

void Kernel::puts(const char* s) {
  _mini_uart.puts(s);
}


extern "C" void kmain(char* bss_start, char* bss_end) {
  // Initialize bss segment to 0
  valkyrie::kernel::memset(bss_start, 0, bss_end - bss_start);
  
  // Run the kernel
  valkyrie::kernel::Kernel kernel;
  kernel.run();
}

}  // namespace valkyrie::kernel

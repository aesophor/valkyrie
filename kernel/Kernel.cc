// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <String.h>

namespace valkyrie::kernel {

Kernel::Kernel() : _mini_uart() {}


void Kernel::run() {
  puts("Valkyrie Operating System");
  puts("=========================");


  // Lab1 SimpleShell
  char buf[256];

  while (true) {
    memset(buf, 0, sizeof(buf));
    _mini_uart.putchar('$');
    _mini_uart.putchar(' ');
    gets(buf);
    puts(buf);

    if (!strcmp(buf, "help")) {
      puts("usage:");
      puts("help   - Print all available commands");
      puts("hello  - Print Hello World!");
      puts("reboot - Reboot machine");
    } else if (!strcmp(buf, "hello")) {
      puts("Hello World!");
    } else if (!strcmp(buf, "reboot")) {
      puts("Rebooting...");
    } else {
      puts("command not found");
    }
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

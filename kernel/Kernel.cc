// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <IO.h>
#include <String.h>

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

namespace valkyrie::kernel {

Kernel::Kernel() : _mini_uart() {}


void Kernel::run() {
  puts("Valkyrie Operating System");
  puts("=========================");


  // Lab1 SimpleShell
  char buf[256];

  while (true) {
    memset(buf, 0, sizeof(buf));
    putchar('$');
    putchar(' ');
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
      reboot();
    } else {
      puts("command not found");
    }
  }

  puts("bye...");
  while (1);
}

void Kernel::reboot() {
  reset(100);
}


void Kernel::gets(char* s) {
  _mini_uart.gets(s);
}

void Kernel::puts(const char* s) {
  _mini_uart.puts(s);
}

void Kernel::putchar(const char c) {
  _mini_uart.putchar(c);
}


void Kernel::reset(int tick) {  // reboot after watchdog timer expire
  io::write(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
  io::write(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void Kernel::cancel_reset() {
  io::write(PM_RSTC, PM_PASSWORD | 0);  // full reset
  io::write(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}


extern "C" void kmain(char* bss_start, char* bss_end) {
  // Initialize bss segment to 0
  valkyrie::kernel::memset(bss_start, 0, bss_end - bss_start);
  
  // Run the kernel
  valkyrie::kernel::Kernel kernel;
  kernel.run();
}

}  // namespace valkyrie::kernel

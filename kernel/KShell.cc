// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <KShell.h>

#include <Console.h>
#include <Power.h>
#include <String.h>

namespace valkyrie::kernel {

KShell::KShell() : _buf() {}

void KShell::run() {
  while (true) {
    memset(_buf, 0, sizeof(_buf));
    putchar('$');
    putchar(' ');
    gets(_buf);
    puts(_buf);

    if (!strcmp(_buf, "help")) {
      puts("usage:");
      puts("help   - Print all available commands");
      puts("hello  - Print Hello World!");
      puts("reboot - Reboot machine");
    } else if (!strcmp(_buf, "hello")) {
      puts("Hello World!");
    } else if (!strcmp(_buf, "reboot")) {
      puts("Rebooting...");
      reset(100);
    } else if (!strcmp(_buf, "loadimg")) {
      puts("Start loading kernel image...");
      //loadimg();
    } else {
      puts("command not found");
    }
  }
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <KShell.h>

#include <Kernel.h>
#include <Console.h>
#include <Power.h>
#include <String.h>

extern "C" void omg(void);

namespace valkyrie::kernel {

KShell::KShell() : _buf() {}

void KShell::run() {
  while (true) {
    memset(_buf, 0, sizeof(_buf));
    putchar('$');
    putchar(' ');
    gets(_buf);

    if (!strlen(_buf)) {
      continue;
    }

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
    } else if (!strcmp(_buf, "exc")) {
      omg();
    } else if (!strcmp(_buf, "panic")) {
      Kernel::get_instance()->panic();
    } else {
      puts("command not found");
    }
  }
}

}  // namespace valkyrie::kernel

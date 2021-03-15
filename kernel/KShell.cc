// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <KShell.h>

#include <Kernel.h>
#include <Console.h>
#include <Power.h>
#include <String.h>

namespace valkyrie::kernel {

KShell::KShell() : _buf() {}

void KShell::run() {
  while (true) {
    memset(_buf, 0, sizeof(_buf));
    printf("root# ");
    gets(_buf);

    if (!strlen(_buf)) {
      continue;
    }

    if (!strcmp(_buf, "help")) {
      puts("usage:");
      puts("help   - Print all available commands");
      puts("hello  - Print Hello World!");
      puts("reboot - Reboot machine");
      puts("exc    - Trigger an exception via supervisor call (SVC)");
      puts("irq    - Execute sys_irq() which enables ARM core timer");
      puts("panic  - Trigger a kernel panic and halt the kernel");
    } else if (!strcmp(_buf, "hello")) {
      puts("Hello World!");
    } else if (!strcmp(_buf, "reboot")) {
      puts("Rebooting...");
      reset(100);
    } else if (!strcmp(_buf, "exc")) {
      asm volatile("svc #1");
    } else if (!strcmp(_buf, "irq")) {
      asm volatile("mov x8, #0");
      asm volatile("mov x0, #1");
      asm volatile("mov x1, #2");
      asm volatile("mov x2, #3");
      asm volatile("mov x3, #4");
      asm volatile("mov x4, #5");
      asm volatile("mov x5, #6");
      asm volatile("svc #0");
    } else if (!strcmp(_buf, "panic")) {
      panic("panic on demand");
    } else {
      printf("%s: command not found. Try <help>\n", _buf);
    }
  }
}

}  // namespace valkyrie::kernel

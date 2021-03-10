// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <KShell.h>

#include <Kernel.h>
#include <Console.h>
#include <Power.h>
#include <String.h>

extern "C" void omg(void);
extern "C" size_t get_sp(void);

namespace valkyrie::kernel {

KShell::KShell() : _buf() {}

void KShell::run() {
  while (true) {
    memset(_buf, 0, sizeof(_buf));
    printf("# ");
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
      puts("panic  - Trigger a kernel panic and halt the kernel");
    } else if (!strcmp(_buf, "hello")) {
      puts("Hello World!");
    } else if (!strcmp(_buf, "reboot")) {
      puts("Rebooting...");
      reset(100);
    } else if (!strcmp(_buf, "exc")) {
      asm volatile("svc 1");
    } else if (!strcmp(_buf, "irq")) {
      printk("ARM core timer enabled.\n");
      Kernel::get_instance()->get_exception_manager()->get_arm_core_timer().enable();
    } else if (!strcmp(_buf, "panic")) {
      panic("panic on demand");
    } else {
      printf("%s: command not found. Try <help>\n", _buf);
    }
  }
}

}  // namespace valkyrie::kernel

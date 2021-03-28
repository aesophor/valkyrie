// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Shell.h>

#include <Kernel.h>
#include <Console.h>
#include <MemoryManager.h>
#include <Power.h>
#include <String.h>

namespace valkyrie::kernel {

Shell::Shell() : _buf() {}

void Shell::run() {
  void* p1 = kmalloc(4080);
  void* p2 = kmalloc(8787);

  kfree(p1);
  kfree(p2);

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
      asm volatile("mov x1, #0");
      asm volatile("svc #0");
    /*
    } else if (!strcmp(_buf, "kmalloc")) {
      printf("how many bytes: ");
      gets(_buf);
      p = MemoryManager::get_instance()->kmalloc(atoi(_buf));
      printf("got pointer 0x%x\n", p);
    } else if (!strcmp(_buf, "kfree")) {
      printf("which pointer to free (in hexadecimal without the 0x prefix): ");
      gets(_buf);
      void* ptr = reinterpret_cast<void*>(atoi(_buf, 16));
      MemoryManager::get_instance()->kfree(ptr);
    */
    } else if (!strcmp(_buf, "panic")) {
      Kernel::panic("panic on demand\n");
    } else {
      printf("%s: command not found. Try <help>\n", _buf);
    }
  }
}

}  // namespace valkyrie::kernel

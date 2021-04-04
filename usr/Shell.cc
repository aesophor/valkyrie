// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <usr/Shell.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <kernel/Power.h>
#include <libs/String.h>
#include <mm/MemoryManager.h>

namespace valkyrie::kernel {

Shell::Shell() : _buf() {}

Shell::~Shell() {
  printf("Shell::~Shell()\n");
}

void Shell::run() {
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

    } else if (!strcmp(_buf, "kmalloc")) {
      printf("how many bytes: ");
      gets(_buf);
      void* p = kmalloc(atoi(_buf));
      printf("got pointer 0x%x\n", p);

    } else if (!strcmp(_buf, "kfree")) {
      printf("which pointer to free (in hexadecimal without the 0x prefix): ");
      gets(_buf);
      void* ptr = reinterpret_cast<void*>(atoi(_buf, 16));
      kfree(ptr);

    } else if (!strcmp(_buf, "buddy_info")) {
      MemoryManager::get_instance()->dump_page_frame_allocator_info();

    } else if (!strcmp(_buf, "slob_info")) {
      MemoryManager::get_instance()->dump_slob_allocator_info();

    } else if (!strcmp(_buf, "panic")) {
      Kernel::panic("panic on demand\n");

    } else if (!strcmp(_buf, "exit")) {
      break;

    } else {
      printf("%s: command not found. Try <help>\n", _buf);
    }
  }
}

}  // namespace valkyrie::kernel

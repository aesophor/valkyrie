// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <usr/Shell.h>

#include <String.h>
#include <dev/Console.h>
#include <fs/ELF.h>
#include <kernel/Kernel.h>
#include <kernel/Power.h>
#include <kernel/TimerMultiplexer.h>
#include <libs/CString.h>
#include <mm/MemoryManager.h>

using namespace valkyrie::kernel;

void run_shell() {
  char _buf[256];

  while (true) {
    memset(_buf, 0, sizeof(_buf));
    printf("root# ");
    gets(_buf);

    if (!strlen(_buf)) {
      continue;
    }

    if (!strcmp(_buf, "help")) {
      puts("usage:");
      puts("help          - Print all available commands");
      puts("reboot        - Reboot machine");
      puts("exc           - Trigger an exception via supervisor call (SVC)");
      puts("timer_enable  - enable the ARM core timer");
      puts("timer_disable - disable the ARM core timer");
      puts("panic         - Trigger a kernel panic and halt the kernel");

    } else if (!strcmp(_buf, "reboot")) {
      puts("Rebooting...");
      reset(100);

    } else if (!strcmp(_buf, "exc")) {
      asm volatile("svc #1");

    } else if (!strcmp(_buf, "timer_enable")) {
      asm volatile("mov x1, #0");
      asm volatile("svc #0");

    } else if (!strcmp(_buf, "timer_disable")) {
      asm volatile("mov x1, #1");
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
      MemoryManager::get_instance().dump_page_frame_allocator_info();

    } else if (!strcmp(_buf, "slob_info")) {
      MemoryManager::get_instance().dump_slob_allocator_info();

    } else if (!strcmp(_buf, "run")) {
      /*
      printf("filename: ");
      gets(_buf);
      String filename = _buf;
 
      size_t filesize = 0;
      const char* base = Initramfs::get_instance().read(_buf, &filesize);
      ELF elf(base, filesize);
      printf("filesize = %d\n", filesize);

      printf("address to load program at: ");
      gets(_buf);
      size_t dest_addr = atoi(_buf, 16);
      void* dest = reinterpret_cast<void*>(dest_addr);
      void* entry = reinterpret_cast<void*>(dest_addr + sizeof(ELF::FileHeader));
      memcpy(dest, base, filesize);

      printf("branching to 0x%x\n", entry);
      void* user_sp = reinterpret_cast<void*>(0x20000);
      ExceptionManager::get_instance().switch_to_exception_level(0, entry, user_sp);
      */
    } else if (!strcmp(_buf, "set_timeout")) {
      printf("message: ");
      gets(_buf);
      String msg = _buf;
      printf("timeout: ");
      gets(_buf);
      
      auto callback = [msg]() { printk("%s\n", msg.c_str()); };
      TimerMultiplexer::get_instance().add_timer(callback, atoi(_buf));

    } else if (!strcmp(_buf, "async_io_enable")) {
      printf("async I/O enabled\n");
      MiniUART::get_instance().set_read_buffer_enabled(true);
      MiniUART::get_instance().set_write_buffer_enabled(true);

    } else if (!strcmp(_buf, "async_io_disable")) {
      printf("async I/O disabled\n");
      MiniUART::get_instance().set_read_buffer_enabled(false);
      MiniUART::get_instance().set_write_buffer_enabled(false);

    } else if (!strcmp(_buf, "async_io_dbg")) {
      MiniUART::get_instance().set_debugging(
          !MiniUART::get_instance().is_debugging());

    } else if (!strcmp(_buf, "panic")) {
      Kernel::panic("panic on demand\n");

    } else if (!strcmp(_buf, "exit")) {
      break;

    } else {
      printf("%s: command not found. Try <help>\n", _buf);
    }
  }
}

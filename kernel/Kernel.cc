// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

namespace {

void run_shell() {
  auto& console = valkyrie::kernel::Console::get_instance();
  char _buf[64] = {};

  while (true) {
    memset(_buf, 0, sizeof(_buf));
    printf("root# ");
    
    console.read(_buf, 63);

    if (!strlen(_buf)) {
      continue;
    }

    if (!strcmp(_buf, "help")) {
      printf("usage:");
      printf("help   - Print all available commands");
      printf("hello  - Print Hello World!");
      printf("reboot - Reboot machine");
      printf("exc    - Trigger an exception via supervisor call (SVC)");
      printf("irq    - Execute sys_irq() which enables ARM core timer");
      printf("panic  - Trigger a kernel panic and halt the kernel");

    } else if (!strcmp(_buf, "a")) {
      printf("how many bytes: ");
      console.read(_buf, 63);

      void* p = kmalloc(atoi(_buf));
      printf("got pointer 0x%x\n", p);

    } else if (!strcmp(_buf, "f")) {
      printf("which pointer to free (in hexadecimal without the 0x prefix): ");
      console.read(_buf, 63);

      void* ptr = reinterpret_cast<void*>(atoi(_buf, 16));
      kfree(ptr);

    } else if (!strcmp(_buf, "info")) {
      valkyrie::kernel::MemoryManager::get_instance().dump_buddy_allocator_info();

    } else {
      printf("%s: command not found. Try <help>\n", _buf);
    }
  }
}

}  // namespace

namespace valkyrie::kernel {

Kernel& Kernel::get_instance() {
  static Kernel instance;
  return instance;
}

Kernel::Kernel()
    : _mailbox(Mailbox::get_instance()),
      _mini_uart(MiniUART::get_instance()),
      _memory_manager(MemoryManager::get_instance()),
      _exception_manager(ExceptionManager::get_instance()),
      _timer_multiplexer(TimerMultiplexer::get_instance()),
      _task_scheduler(TaskScheduler::get_instance()),
      _vfs(VFS::get_instance()) {}


void Kernel::run() {
  print_banner();
  print_hardware_info();

  /*
  printk("enabling timer interrupts\n");
  _timer_multiplexer.get_arm_core_timer().enable();
  */
 
  printk("VFS: mounting rootfs\n");
  _vfs.mount_rootfs();

  printk("VFS: mounting devtmpfs\n");
  _vfs.mount_devtmpfs();
  _vfs.populate_devtmpfs();

  printk("starting task scheduler\n");
  _task_scheduler.run();

  run_shell();

  Kernel::panic("you shouldn't have reached here...\n");
}


void Kernel::print_banner() {
  auto& console = Console::get_instance();

  console.set_color(Console::Color::GREEN, /*bold=*/true);
  printf("--- Valkyrie OS (Virtual Memory Edition) ---\n");
  console.set_color(Console::Color::YELLOW, /*bold=*/true);
  printf("Developed by: Marco Wang <aesophor.cs09g@nctu.edu.tw>\n\n");
  console.clear_color();
}

void Kernel::print_hardware_info() {
  const auto board_revision = _mailbox.get_board_revision();
  const auto vc_memory_info = _mailbox.get_vc_memory();

  printk("Hardware: Raspberry Pi 3B+ (revision: %x)\n", board_revision);
  printk("RAM size: 0x%x\n", _memory_manager.get_ram_size());
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);
}

}  // namespace valkyrie::kernel

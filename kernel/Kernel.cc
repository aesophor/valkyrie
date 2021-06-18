// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

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

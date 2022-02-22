// Copyright (c) 2021-2022 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

Kernel::Kernel()
    : _mailbox(Mailbox::the()),
      _mini_uart(MiniUART::the()),
      _console(Console::the()),
      _memory_manager(MemoryManager::the()),
      _exception_manager(ExceptionManager::the()),
      _timer_multiplexer(TimerMultiplexer::the()),
      _task_scheduler(TaskScheduler::the()),
      _vfs(VFS::the()) {}


void Kernel::run() {
  print_banner();
  print_hardware_info();

  printk("VFS: initializing\n");
  _vfs.mount_rootfs();
  _vfs.mount_devtmpfs();
  _vfs.mount_procfs();
  _vfs.populate_devtmpfs();

  //printk("enabling timer interrupts\n");
  //_exception_manager.enableIRQs();
  //_timer_multiplexer.get_arm_core_timer().enable();

  printk("starting task scheduler\n");
  _task_scheduler.run();

  Kernel::panic("you shouldn't have reached here...\n");
}


void Kernel::print_banner() {
  auto& console = Console::the();
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

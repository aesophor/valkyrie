// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

#include <proc/Scheduler.h>
#include <usr/Shell.h>

extern "C" [[noreturn]] void _halt(void);

namespace valkyrie::kernel {

extern void __run_unit_tests();

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
      _initramfs() {}


void Kernel::run() {
  //_mini_uart.set_read_buffer_enabled(true);
  //_mini_uart.set_write_buffer_enabled(true);

  print_banner();
  print_hardware_info();

  printk("switching to supervisor mode... (≧▽ ≦)\n");
  _exception_manager.switch_to_exception_level(1);
  _exception_manager.enable();

  // Run some unit tests...
  //__run_unit_tests();

  //printk("switching to user mode... (≧▽ ≦)\n");
  //_exception_manager.switch_to_exception_level(0, /*new_sp=*/0x20000);

  // Lab1 SimpleShell
  Shell().run();

  printk("you shouldn't have reached here :(\n");
  _halt();
}


void Kernel::print_banner() {
  console::set_color(console::Color::GREEN, /*bold=*/true);
  puts("--- Valkyrie OS ---");
  console::set_color(console::Color::YELLOW, /*bold=*/true);
  puts("Developed by: Marco Wang <aesophor.cs09g@nctu.edu.tw>");
  console::clear_color();
}

void Kernel::print_hardware_info() {
  const auto board_revision = _mailbox.get_board_revision();
  const auto arm_memory_info = _mailbox.get_arm_memory();
  const auto vc_memory_info = _mailbox.get_vc_memory();

  printk("Hardware: Raspberry Pi 3B+ (revision: %x)\n", board_revision);
  printk("ARM memory size: 0x%x\n", arm_memory_info.second);
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);
}


Initramfs& Kernel::get_initramfs() {
  return _initramfs;
}

}  // namespace valkyrie::kernel

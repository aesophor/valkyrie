// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

#include <Memory.h>
#include <dev/Console.h>
#include <usr/Shell.h>

extern "C" [[noreturn]] void _halt(void);

namespace valkyrie::kernel {

Kernel* Kernel::get_instance() {
  static Kernel instance;
  return &instance;
}

Kernel::Kernel()
    : _mini_uart(),
      _mailbox(),
      _initrd_cpio(CPIO_BASE),
      _exception_manager(*ExceptionManager::get_instance()),
      _memory_manager(*MemoryManager::get_instance()) {}


void Kernel::run() {
  print_banner();
  print_hardware_info();

  printk("initializing initramfs at 0x%x\n", CPIO_BASE);
  //_initrd_cpio.parse();

  printk("switching to supervisor mode...\n");
  _exception_manager.switch_to_exception_level(1);
  _exception_manager.enable();

  printk("switching to user mode... (≧▽ ≦)\n");
  _exception_manager.switch_to_exception_level(0, /*new_sp=*/0x20000);

  // Lab1 SimpleShell
  auto shell = make_unique<Shell[]>(2);
  shell[0].run();
  //shell->run();

  printf("you shouldn't have reached here :(\n");
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
  const auto vc_memory_info = _mailbox.get_vc_memory();

  printk("board revision: 0x%x\n", board_revision);
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);
}

}  // namespace valkyrie::kernel

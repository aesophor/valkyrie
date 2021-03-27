// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <Console.h>
#include <Shell.h>
#include <Task.h>

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

  printk("parsing cpio archive at 0x%x\n", CPIO_BASE);
  //_initrd_cpio.parse();

  printk("switching to EL1...\n");
  _exception_manager.switch_to_exception_level(1);

  printk("switching to EL0... (≧▽ ≦)\n");
  _exception_manager.switch_to_exception_level(0, /*new_sp=*/0x20000);

  // Lab1 SimpleShell
  Shell shell;
  shell.run();
}


void Kernel::print_banner() {
  puts("\033[1;32m", /*newline=*/false);
  puts("--- Valkyrie OS ---");
  puts("\033[1;33m", /*newline=*/false);
  puts("Developed by: Marco Wang <aesophor.cs09g@nctu.edu.tw>");
  puts("\033[0m\n", /*newline=*/false);
}

void Kernel::print_hardware_info() {
  const auto board_revision = _mailbox.get_board_revision();
  const auto vc_memory_info = _mailbox.get_vc_memory();

  printk("board revision: 0x%x\n", board_revision);
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);
}

}  // namespace valkyrie::kernel

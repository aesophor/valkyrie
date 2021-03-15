// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <Console.h>
#include <Shell.h>
#include <Task.h>

extern "C" [[noreturn]] void _halt(void);

namespace valkyrie::kernel {

Kernel* Kernel::get_instance() {
  static Kernel instance;
  return &instance;
}

Kernel::Kernel()
    : _mini_uart(),
      _mailbox(),
      _initrd_cpio(reinterpret_cast<const char*>(CPIO_BASE)),
      _exception_manager(*ExceptionManager::get_instance()) {
  printk("starting valkyrie OS...\n");
  print_hardware_info();

  printk("parsing cpio archive at 0x%x\n", CPIO_BASE);
  _initrd_cpio.parse();

  //printk("switching to EL1...\n");
  //_exception_manager.switch_to_exception_level(1);
  //_exception_manager.enable_irqs();
}


void Kernel::run() {
  //printk("switching to EL0... (≧▽ ≦) \n");
  //_exception_manager.switch_to_exception_level(0, /*new_sp=*/0x20000);
  //asm volatile("mov sp, %0" :: "r" (0x20000));

  // Lab1 SimpleShell
  Shell shell;
  shell.run();
}

void Kernel::panic(const char* msg) {
  printk("Kernel panic: %s\n", msg);
  printk("---[ end Kernel panic: %s\n", msg);
  _halt();
}

void Kernel::print_hardware_info() {
  auto board_revision = _mailbox.get_board_revision();
  printk("board revision: 0x%x\n", board_revision);

  auto vc_memory_info = _mailbox.get_vc_memory();
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);
}


ExceptionManager* Kernel::get_exception_manager() {
  return &_exception_manager;
}

}  // namespace valkyrie::kernel

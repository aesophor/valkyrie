// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <libs/printf.h>
#include <Console.h>
#include <CPIO.h>
#include <KShell.h>
#include <String.h>
#include <Utility.h>

extern "C" [[noreturn]] void _halt(void);

namespace valkyrie::kernel {

Kernel* Kernel::get_instance() {
  static Kernel instance;
  return &instance;
}

Kernel::Kernel()
    : _mailbox(),
      _mini_uart(),
      _interruptManager() {
  console::initialize(&_mini_uart);
}


void Kernel::run() {
  printk("valkyrie by @aesophor\n");

  printk("current exception level: %d\n",
         _interruptManager.get_current_exception_level());

  auto board_revision = _mailbox.get_board_revision();
  printk("board revision: 0x%x\n", board_revision);

  auto vc_memory_info = _mailbox.get_vc_memory();
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);

  // Lab2 initramfs
  //CPIO cpio(reinterpret_cast<char*>(CPIO_BASE));

  // Lab1 SimpleShell
  KShell shell;
  shell.run();
}

void Kernel::panic() {
  printk("Kernel panic - not syncing!\n");
  _halt();
}

}  // namespace valkyrie::kernel

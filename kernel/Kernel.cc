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
    : _mini_uart(),
      _mailbox(),
      _interruptManager(),
      _initrd_cpio(reinterpret_cast<const char*>(CPIO_BASE)) {
  printk("valkyrie by @aesophor\n");

  printk("current exception level: %d\n",
         _interruptManager.get_current_exception_level());

  auto board_revision = _mailbox.get_board_revision();
  printk("board revision: 0x%x\n", board_revision);

  auto vc_memory_info = _mailbox.get_vc_memory();
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);

  _initrd_cpio.parse();
}


void Kernel::run() {
  // Lab1 SimpleShell
  KShell shell;
  shell.run();
}

void Kernel::panic() {
  printk("Kernel panic - not syncing!\n");
  _halt();
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <Console.h>
#include <KShell.h>
#include <String.h>
#include <Utility.h>

namespace valkyrie::kernel {

Kernel* Kernel::get_instance() {
  static Kernel instance;
  return &instance;
}

Kernel::Kernel()
    : _mailbox(),
      _mini_uart() {
  console_init(&_mini_uart);
}


void Kernel::run() {
  puts("valkyrie v0.1 by @aesophor\n");
  puts("[*] board revision: ", false);
  print_hex(_mailbox.get_board_revision());

  auto vc_memory_info = _mailbox.get_vc_memory();
  puts("[*] vc core base address: ", false);
  print_hex(vc_memory_info.first);
  puts("[*] vc core size: ", false);
  print_hex(vc_memory_info.second);

  // Lab1 SimpleShell
  KShell shell;
  shell.run();
}

}  // namespace valkyrie::kernel

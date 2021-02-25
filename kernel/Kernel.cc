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

Kernel::Kernel() : _mailbox(), _mini_uart() {}


void Kernel::run() {
  console_init(&_mini_uart);

  puts("Valkyrie Operating System");
  puts("=========================");
  puts("board revision:");
  print_hex(_mailbox.get_board_revision());

  auto vc_memory_info = _mailbox.get_vc_memory();
  puts("vc core base address / size:");
  print_hex(vc_memory_info.first);
  print_hex(vc_memory_info.second);

  // Lab1 SimpleShell
  KShell shell;
  shell.run();
}

}  // namespace valkyrie::kernel


extern "C"
[[noreturn]] void kmain(char* bss_start, char* bss_end) {
  // Initialize bss segment to 0
  valkyrie::kernel::memset(bss_start, 0, bss_end - bss_start);

  // Run the kernel
  valkyrie::kernel::Kernel::get_instance()->run();

  while (1);
}


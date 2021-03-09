// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <libs/printf.h>
#include <Bootloader.h>
#include <Console.h>
#include <KShell.h>
#include <String.h>
#include <Utility.h>

extern "C" uint32_t get_cntfrq_el0(void);
extern "C" uint32_t get_cntpct_el0(void);

namespace valkyrie::kernel {

Kernel* Kernel::get_instance() {
  static Kernel instance;
  return &instance;
}

Kernel::Kernel()
//    : _mailbox(),
 :     _mini_uart() {
//      _interruptManager() {
  console::initialize(&_mini_uart);
}


void Kernel::run() {
  printf("[valkyrie bootloader] by @aesophor\n");

  //printf("[*] current exception level: %d\n",
  //       _interruptManager.get_current_exception_level());

  /*
  auto board_revision = _mailbox.get_board_revision();
  printf("[*] board revision: 0x%x\n", board_revision);

  auto vc_memory_info = _mailbox.get_vc_memory();
  printf("[*] VC core base address: 0x%x\n", vc_memory_info.first);
  printf("[*] VC core size: 0x%x\n", vc_memory_info.second);
  */

  // Lab2 Bootloader
  Bootloader bootloader;
  bootloader.run();

  // Lab1 SimpleShell
  //KShell shell;
  //shell.run();
}


uint32_t Kernel::get_timestamp() const {
  return 1000 * get_cntpct_el0() / get_cntfrq_el0();
}

}  // namespace valkyrie::kernel

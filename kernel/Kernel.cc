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

Kernel::Kernel() : _mini_uart() {
  console::initialize(&_mini_uart);
}


void Kernel::run() {
  printf("[valkyrie bootloader] by @aesophor\n");

  // Lab2 Bootloader
  Bootloader bootloader;
  bootloader.run();
}


uint32_t Kernel::get_timestamp() const {
  return 1000 * get_cntpct_el0() / get_cntfrq_el0();
}

}  // namespace valkyrie::kernel

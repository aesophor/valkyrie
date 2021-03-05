// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Bootloader.h>

#include <Console.h>
#include <IO.h>
#include <String.h>

namespace valkyrie::kernel {

Bootloader::Bootloader() : _kernel_size(), _buf() {}


void Bootloader::prompt_kernel_size() {
  printf("Kernel size: ");
  gets(_buf);
  _kernel_size = atoi(_buf);
}

void Bootloader::prompt_kernel_binary_and_load() {
  printf("Send kernel binary: ");
  
  // Receive kernel image byte by byte through miniUART.
  size_t addr = KERNEL_BASE_ADDR;
  for (size_t i = 0; i < _kernel_size; i++) {
    io::write(addr++, getchar());
  }

  // Jump to the kernel and start executing there.
  reinterpret_cast<void (*)(void)>(KERNEL_BASE_ADDR)();
}

}  // namespace valkyrie::kernel

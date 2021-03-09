// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Bootloader.h>

#include <Console.h>
#include <IO.h>
#include <String.h>

namespace valkyrie::kernel {

Bootloader::Bootloader() : _kernel_size(), _buf() {}


void Bootloader::run() {
  puts("Now please run scripts/send.sh <kernel8.img>");
  prompt_kernel_size();
  prompt_kernel_binary_and_load();
}

void Bootloader::prompt_kernel_size() {
  gets(_buf);
  _kernel_size = atoi(_buf);
}

void Bootloader::prompt_kernel_binary_and_load() {
  // Receive kernel image byte by byte through miniUART.
  size_t addr = KERNEL_BASE_ADDR;
  for (size_t i = 0; i < _kernel_size; i++) {
    io::put<uint8_t>(addr++, _recv());
  }

  printf("Your kernel8.img has been written to 0x%x\n", KERNEL_BASE_ADDR);
  puts("Starting kernel...\n");
  io::delay(1000);  // wait for all output to be printed

  // Jump to the kernel and start executing there.
  reinterpret_cast<void (*)()>(KERNEL_BASE_ADDR)();
}

}  // namespace valkyrie::kernel

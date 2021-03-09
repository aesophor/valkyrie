// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_BOOTLOADER_H_
#define VALKYRIE_BOOTLOADER_H_

#include <Types.h>

#define KERNEL_BASE_ADDR 0x80000
#define BUF_SIZE 16

namespace valkyrie::kernel {

class Bootloader {
 public:
  Bootloader();
  ~Bootloader() = default;

  void run();

 private:
  void prompt_kernel_size();
  void prompt_kernel_binary_and_load();

  size_t _kernel_size;
  char _buf[BUF_SIZE];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_BOOTLOADER_H_

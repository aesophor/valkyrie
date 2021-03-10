// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <CPIO.h>
#include <Mailbox.h>
#include <MiniUART.h>
#include <InterruptManager.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel* get_instance();
  ~Kernel() = default;

  void run();
  [[noreturn]] void panic();

 private:
  Kernel();

  MiniUART _mini_uart;
  Mailbox _mailbox;
  InterruptManager _interruptManager;
  CPIO _initrd_cpio;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

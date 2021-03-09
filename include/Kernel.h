// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <Mailbox.h>
#include <MiniUART.h>
#include <InterruptManager.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel* get_instance();
  ~Kernel() = default;

  void run();
  uint32_t get_timestamp() const;

 private:
  Kernel();

  MiniUART _mini_uart;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

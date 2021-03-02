// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <Mailbox.h>
#include <MiniUART.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel* get_instance();
  ~Kernel() = default;

  void run();
  MiniUART* get_mini_uart() { return &_mini_uart; }

 private:
  Kernel();

  void reset(int tick);
  void cancel_reset();

  Mailbox _mailbox;
  MiniUART _mini_uart;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

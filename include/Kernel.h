// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <MiniUART.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  Kernel();
  ~Kernel() = default;

  void run();
  void reboot();

 private:
  void gets(char* s);
  void puts(const char* s);
  void putchar(const char c);

  void reset(int tick);
  void cancel_reset();

  MiniUART _mini_uart;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

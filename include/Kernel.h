// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <CPIO.h>
#include <Mailbox.h>
#include <MiniUART.h>
#include <ExceptionManager.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel* get_instance();
  ~Kernel() = default;

  void run();
  [[noreturn]] void panic(const char* msg);

  ExceptionManager* get_exception_manager();

 private:
  Kernel();

  void print_hardware_info();

  MiniUART _mini_uart;
  Mailbox _mailbox;
  CPIO _initrd_cpio;
  ExceptionManager& _exception_manager;
};

}  // namespace valkyrie::kernel


extern "C" [[noreturn]] inline void panic(const char* msg) {
  valkyrie::kernel::Kernel::get_instance()->panic(msg);
}

#endif  // VALKYRIE_KERNEL_H_

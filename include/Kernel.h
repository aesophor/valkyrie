// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <Console.h>
#include <CPIO.h>
#include <Mailbox.h>
#include <MiniUART.h>
#include <ExceptionManager.h>
#include <MemoryManager.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel* get_instance();
  ~Kernel() = default;

  void run();

  template <typename... Args>
  [[noreturn]] static void panic(char* fmt, Args&&... args);

 private:
  Kernel();

  void print_banner();
  void print_hardware_info();

  MiniUART _mini_uart;
  Mailbox _mailbox;
  CPIO _initrd_cpio;
  ExceptionManager& _exception_manager;
  MemoryManager& _memory_manager;
};


extern "C" [[noreturn]] void _halt(void);

template <typename... Args>
[[noreturn]] void Kernel::panic(char* fmt, Args&&... args) {
  printk("\033[1;31mKernel panic: \033[0;33m");
  printf(fmt, args...);
  puts("\033[0m", /*newline=*/false);

  printk("\033[1;31m---[ end Kernel panic: \033[0;33m");
  printf(fmt, args...);
  puts("\033[0m", /*newline=*/false);

  _halt();
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

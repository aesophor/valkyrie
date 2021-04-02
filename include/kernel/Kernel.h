// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <dev/Console.h>
#include <dev/Mailbox.h>
#include <dev/MiniUART.h>
#include <fs/CPIO.h>
#include <kernel/ExceptionManager.h>
#include <mm/MemoryManager.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel* get_instance();
  ~Kernel() = default;

  [[noreturn]] void run();

  template <typename... Args>
  [[noreturn]] static void panic(const char* fmt, Args&&... args);

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
[[noreturn]] void Kernel::panic(const char* fmt, Args&&... args) {
  console::clear_color();
  printk("");
  console::set_color(console::Color::RED, /*bold=*/true);
  printf("Kernel panic: ");
  console::set_color(console::Color::YELLOW);
  printf(fmt, args...);

  console::clear_color();
  printk("");
  console::set_color(console::Color::RED, /*bold=*/true);
  printf("---[ end Kernel panic: ");
  console::set_color(console::Color::YELLOW);
  printf(fmt, args...);
  console::clear_color();

  ExceptionManager::get_instance()->disable();
  _halt();
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

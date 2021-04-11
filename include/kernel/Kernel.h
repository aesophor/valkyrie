// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <dev/Console.h>
#include <dev/Mailbox.h>
#include <dev/MiniUART.h>
#include <fs/Initramfs.h>
#include <kernel/ExceptionManager.h>
#include <kernel/TimerMultiplexer.h>
#include <mm/MemoryManager.h>

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel* get_instance();
  ~Kernel() = default;

  [[noreturn]] void run();

  template <typename... Args>
  [[noreturn]] static void panic(const char* fmt, Args&&... args);

  Initramfs& get_initramfs();

 private:
  Kernel();

  void print_banner();
  void print_hardware_info();

  ExceptionManager& _exception_manager;
  MemoryManager& _memory_manager;
  MiniUART& _mini_uart;
  TimerMultiplexer& _timer_multiplexer;
  Mailbox& _mailbox;
  Initramfs _initramfs;
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

  ExceptionManager::get_instance().disable();
  _halt();
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

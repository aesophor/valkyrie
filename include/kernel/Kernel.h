// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <dev/Console.h>
#include <dev/Mailbox.h>
#include <dev/MiniUART.h>
#include <fs/TmpFS.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/ExceptionManager.h>
#include <kernel/TimerMultiplexer.h>
#include <mm/MemoryManager.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

static const char* kernel_panic_msg = "Kernel panic - not syncing: ";

namespace valkyrie::kernel {

class Kernel {
 public:
  static Kernel& get_instance();
  ~Kernel() = default;

  [[noreturn]] void run();

  template <typename... Args>
  [[noreturn]] static void panic(const char* fmt, Args&&... args);

 private:
  Kernel();

  void print_banner();
  void print_hardware_info();

  Mailbox& _mailbox;
  MiniUART& _mini_uart;
  MemoryManager& _memory_manager;
  ExceptionManager& _exception_manager;
  TimerMultiplexer& _timer_multiplexer;
  TaskScheduler& _task_scheduler;
  VirtualFileSystem& _vfs;
};


extern "C" [[noreturn]] void _halt(void);

template <typename... Args>
[[noreturn]] void Kernel::panic(const char* fmt, Args&&... args) {
  uint64_t stack_pointer;
  asm volatile("mov %0, sp" : "=r" (stack_pointer));

  console::clear_color();
  printk("");
  console::set_color(console::Color::RED, /*bold=*/true);
  printf(kernel_panic_msg);
  console::set_color(console::Color::YELLOW);
  printf(fmt, forward<Args>(args)...);
  console::clear_color();

  printk("SP = 0x%x ", stack_pointer);
  if (Task::current()) {
    printf("PID = %d", Task::current()->get_pid());
  }
  puts("");


  MemoryManager::get_instance().dump_slob_allocator_info();

  printk("");
  console::set_color(console::Color::RED, /*bold=*/true);
  printf("---[ end ");
  printf(kernel_panic_msg);
  console::set_color(console::Color::YELLOW);
  printf(fmt, forward<Args>(args)...);
  console::clear_color();

  ExceptionManager::get_instance().disable();
  _halt();
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

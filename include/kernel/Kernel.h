// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <Config.h>
#include <Mutex.h>
#include <Singleton.h>

#include <dev/Console.h>
#include <driver/Mailbox.h>
#include <driver/MiniUART.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/Exception.h>
#include <kernel/TimerMultiplexer.h>
#include <mm/MemoryManager.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

class Kernel : public Singleton<Kernel> {
 public:
  [[noreturn]] void run();

  template <typename... Args>
  [[noreturn]] static void panic(const char *fmt, Args &&...args);

  [[noreturn]] static void halt() {
    for (;;) {
      asm volatile("wfi");
    }
  }

  static constexpr const char *panic_msg = "Kernel panic - not syncing: ";
  static RecursiveMutex mutex;

 protected:
  Kernel();

 private:
  void print_banner();
  void print_hardware_info();

  Mailbox &_mailbox;
  MiniUART &_mini_uart;
  Console &_console;
  TimerMultiplexer &_timer_multiplexer;
  MemoryManager &_memory_manager;
  TaskScheduler &_task_scheduler;
  VFS &_vfs;
};

template <typename... Args>
[[noreturn]] void Kernel::panic(const char *fmt, Args &&...args) {
  exception::disable_irqs();

  uint64_t sp;
  asm volatile("mov %0, sp" : "=r"(sp));

  switch_user_va_space(nullptr);

  auto &console = Console::the();
  console.clear_color();
  printk("");
  console.set_color(Console::Color::RED, /*bold=*/true);
  printf(Kernel::panic_msg);
  console.set_color(Console::Color::YELLOW);
  printf(fmt, forward<Args>(args)...);
  console.clear_color();

  void *ttbr0_el1;
  asm volatile("mrs %0, ttbr0_el1" : "=r"(ttbr0_el1));

  printk("");
  auto task = Task::current();
  printf("Current task: pid %d [%s], ttbr0_el1 = 0x%p, kernel sp = 0x%p\n", task->get_pid(),
         task->get_name(), ttbr0_el1, sp);

  console.clear_color();
  printk("");
  console.set_color(Console::Color::RED, /*bold=*/true);
  printf("---[ end ");
  printf(Kernel::panic_msg);
  console.set_color(Console::Color::YELLOW);
  printf(fmt, forward<Args>(args)...);
  console.clear_color();

  Kernel::halt();
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

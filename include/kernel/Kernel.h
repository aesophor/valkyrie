// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KERNEL_H_
#define VALKYRIE_KERNEL_H_

#include <Config.h>
#include <Singleton.h>
#include <dev/Console.h>
#include <driver/Mailbox.h>
#include <driver/MiniUART.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/ExceptionManager.h>
#include <kernel/TimerMultiplexer.h>
#include <mm/MemoryManager.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

class Kernel : public Singleton<Kernel> {
 public:
  [[noreturn]] void run();

  template <typename... Args>
  [[noreturn]] static void panic(const char* fmt, Args&&... args);

  static constexpr const char* panic_msg = "Kernel panic - not syncing: ";

 protected:
  Kernel();

 private:
  void print_banner();
  void print_hardware_info();

  Mailbox& _mailbox;
  MiniUART& _mini_uart;
  Console& _console;
  ExceptionManager& _exception_manager;
  TimerMultiplexer& _timer_multiplexer;
  MemoryManager& _memory_manager;
  TaskScheduler& _task_scheduler;
  VFS& _vfs;
};


extern "C" [[noreturn]] void _halt(void);

template <typename... Args>
[[noreturn]] void Kernel::panic(const char* fmt, Args&&... args) {
  ExceptionManager::the().disableIRQs();

  auto& console = Console::the();

  uint64_t stack_pointer;
  asm volatile("mov %0, sp" : "=r" (stack_pointer));

  console.clear_color();
  printk("");
  console.set_color(Console::Color::RED, /*bold=*/true);
  printf(Kernel::panic_msg);
  console.set_color(Console::Color::YELLOW);
  printf(fmt, forward<Args>(args)...);
  console.clear_color();

  printk("SP = 0x%x ", stack_pointer);
  /*
  if (Task::current()) {
    printf("PID = %d", Task::current()->get_pid());
  }
  */
  printf("\n");


  MemoryManager::the().dump_buddy_allocator_info();

  printk("");
  console.set_color(Console::Color::RED, /*bold=*/true);
  printf("---[ end ");
  printf(Kernel::panic_msg);
  console.set_color(Console::Color::YELLOW);
  printf(fmt, forward<Args>(args)...);
  console.clear_color();

  _halt();
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KERNEL_H_

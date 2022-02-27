// Copyright (c) 2021-2022 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

#include <proc/idle.h>
#include <proc/start_init.h>
#include <proc/start_kthreadd.h>

namespace valkyrie::kernel {

RecursiveMutex Kernel::mutex;

Kernel::Kernel()
    : _mailbox(Mailbox::the()),
      _mini_uart(MiniUART::the()),
      _console(Console::the()),
      _exception_manager(ExceptionManager::the()),
      _timer_multiplexer(TimerMultiplexer::the()),
      _memory_manager(MemoryManager::the()),
      _task_scheduler(TaskScheduler::the()),
      _vfs(VFS::the()) {}

void Kernel::run() {
  print_banner();
  print_hardware_info();

  printk("VFS: initializing\n");
  _vfs.mount_rootfs();
  _vfs.mount_devtmpfs();
  _vfs.mount_procfs();
  _vfs.populate_devtmpfs();

  printk("Creating initial tasks\n");
  _task_scheduler.enqueue_task(make_kernel_task(nullptr, idle, "idle"));
  _task_scheduler.enqueue_task(make_user_task(nullptr, start_init, "start_init"));
  _task_scheduler.enqueue_task(make_kernel_task(nullptr, start_kthreadd, "start_kthreadd"));

  printk("Enabling timer interrupts\n");
  _timer_multiplexer.get_arm_core_timer().enable();

  printk("Activating exception manager\n");
  _exception_manager.activate();

  printk("Starting task scheduler\n\n");
  _task_scheduler.run();

  Kernel::panic("You shouldn't have reached here...\n");
}

void Kernel::print_banner() {
  _console.set_color(Console::Color::GREEN, /*bold=*/true);
  printf("--- Valkyrie OS (Virtual Memory Edition) ---\n");
  _console.set_color(Console::Color::YELLOW, /*bold=*/true);
  printf("Developed by: Marco Wang <aesophor.cs09g@nctu.edu.tw>\n\n");
  _console.clear_color();
}

void Kernel::print_hardware_info() {
  const auto board_revision = _mailbox.get_board_revision();
  const auto vc_memory_info = _mailbox.get_vc_memory();

  printk("Hardware: Raspberry Pi 3B+ (revision: %x)\n", board_revision);
  printk("RAM size: 0x%x\n", _memory_manager.get_ram_size());
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);
}

}  // namespace valkyrie::kernel

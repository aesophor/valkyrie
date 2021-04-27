// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Syscall.h>

#include <dev/Console.h>
#include <kernel/TimerMultiplexer.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

#define SYSCALL_DECL(func) \
  reinterpret_cast<size_t>(func)

const size_t __syscall_table[__NR_syscall] = {
  SYSCALL_DECL(sys_uart_read),
  SYSCALL_DECL(sys_uart_write),
  SYSCALL_DECL(sys_uart_putchar),
  SYSCALL_DECL(sys_fork),
  SYSCALL_DECL(sys_exec),
  SYSCALL_DECL(sys_exit),
  SYSCALL_DECL(sys_getpid),
  SYSCALL_DECL(sys_timer_irq_enable),
  SYSCALL_DECL(sys_timer_irq_disable),
};


size_t sys_uart_read(char buf[], size_t size) {
  for (size_t i = 0; i < size; i++) {
    buf[i] = MiniUART::get_instance().getchar();
  }
  return size;
}

size_t sys_uart_write(const char buf[], size_t size) {
  for (size_t i = 0; i < size; i++) {
    MiniUART::get_instance().putchar(buf[i]);
  }
  return size;
}

void sys_uart_putchar(const char c) {
  MiniUART::get_instance().putchar(c);
}

int sys_fork() {
  return Task::get_current().fork();
}

int sys_exec(const char* name, const char* const argv[]) {
  return Task::get_current().exec(name, argv);
}

[[noreturn]] void sys_exit() {
  Task::get_current().exit();
}

int sys_getpid() {
  return Task::get_current().get_pid();
}

void sys_timer_irq_enable() {
  printk("ARM core timer enabled.\n");
  TimerMultiplexer::get_instance().get_arm_core_timer().enable();
}

void sys_timer_irq_disable() {
  printk("ARM core timer disabled.\n");
  TimerMultiplexer::get_instance().get_arm_core_timer().disable();
}

}  // namespace valkyrie::kernel

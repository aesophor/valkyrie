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
  SYSCALL_DECL(sys_fork),
  SYSCALL_DECL(sys_exec),
  SYSCALL_DECL(sys_exit),
  SYSCALL_DECL(sys_getpid),
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

int sys_fork() {

}

int sys_exec(const char* name, char *const argv[]) {

}

void sys_exit() {
  // Mark the current task as a zombie process,
  // remove it from the runqueue and add it to the zombie list.
  TaskScheduler::get_instance().mark_as_zombie(Task::get_current());
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

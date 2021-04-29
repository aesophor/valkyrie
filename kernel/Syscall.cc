// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Syscall.h>

#include <dev/Console.h>
#include <kernel/TimerMultiplexer.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

#define SYSCALL_DECL(func) \
  reinterpret_cast<const size_t>(func)

const size_t __syscall_table[Syscall::__NR_syscall] = {
  SYSCALL_DECL(sys_uart_read),
  SYSCALL_DECL(sys_uart_write),
  SYSCALL_DECL(sys_uart_putchar),
  SYSCALL_DECL(sys_fork),
  SYSCALL_DECL(sys_exec),
  SYSCALL_DECL(sys_exit),
  SYSCALL_DECL(sys_getpid),
  SYSCALL_DECL(sys_wait),
  SYSCALL_DECL(sys_sched_yield),
  SYSCALL_DECL(sys_kill),
  SYSCALL_DECL(sys_signal),
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
  return Task::get_current().do_fork();
}

int sys_exec(const char* name, const char* const argv[]) {
  return Task::get_current().do_exec(name, argv);
}

int sys_wait(int* wstatus) {
  return Task::get_current().do_wait(wstatus);
}

[[noreturn]] void sys_exit(int error_code) {
  Task::get_current().do_exit(error_code);
}

int sys_getpid() {
  return Task::get_current().get_pid();
}

int sys_sched_yield() {
  TaskScheduler::get_instance().schedule();
  return 0;  // always succeeds.
}

long sys_kill(pid_t pid, int signal) {
  return Task::get_current().do_kill(pid, static_cast<Signal>(signal));
}

int sys_signal(int signal, void (*handler)()) {
  return Task::get_current().do_signal(signal, handler);
}

}  // namespace valkyrie::kernel

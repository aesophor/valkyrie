// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SYSCALL_H_
#define VALKYRIE_SYSCALL_H_

#include <Types.h>
#include <Utility.h>
#include <dev/Console.h>
#include <kernel/Compiler.h>
#include <proc/TrapFrame.h>

namespace valkyrie::kernel {

enum Syscall {
  SYS_READ,
  SYS_WRITE,
  SYS_OPEN,
  SYS_CLOSE,
  SYS_UART_READ,
  SYS_UART_WRITE,
  SYS_UART_PUTCHAR,
  SYS_FORK,
  SYS_EXEC,
  SYS_EXIT,
  SYS_GETPID,
  SYS_WAIT,
  SYS_SCHED_YIELD,
  SYS_KILL,
  SYS_SIGNAL,
  __NR_syscall
};

// System call table
extern const size_t __syscall_table[Syscall::__NR_syscall];

// Individual system call declaration.
int sys_read(int fd, void* buf, size_t count);
int sys_write(int fd, const void* buf, size_t count);
int sys_open(const char* pathname, int options);
int sys_close(int fd);
size_t sys_uart_read(char buf[], size_t size);
size_t sys_uart_write(const char buf[], size_t size);
void sys_uart_putchar(const char c);
int sys_fork();
int sys_exec(const char* name, const char *const argv[]);
[[noreturn]] void sys_exit(int error_code);
int sys_getpid();
int sys_wait(int* wstatus);
int sys_sched_yield();
long sys_kill(pid_t pid, int signal);
int sys_signal(int signal, void (*handler)());


inline bool is_syscall_id_valid(const uint64_t id) {
  return id < Syscall::__NR_syscall;
}

// Indirect system call
//
// A system call is issued using the `svc 0` instruction.
// The system call number is passed via register x8,
// the parameters are stored in x0 ~ x5,
// and the return value will be stored in x0.
template <typename... Args>
size_t do_syscall(const uint64_t id, Args... args) {
  // Check if syscall id is valid.
  if (unlikely(!is_syscall_id_valid(id))) {
    printk("bad system call: (id=0x%x)\n", id);
    return -1;
  }

  // Invoke the specified system call.
  return reinterpret_cast<size_t (*)(Args...)>(__syscall_table[id])(args...);
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SYSCALL_H_

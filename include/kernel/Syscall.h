// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SYSCALL_H_
#define VALKYRIE_SYSCALL_H_

#include <Types.h>
#include <dev/Console.h>
#include <kernel/Compiler.h>
#include <proc/TrapFrame.h>

namespace valkyrie::kernel {

enum Syscall {
  SYS_UART_READ,
  SYS_UART_WRITE,
  SYS_UART_PUTCHAR,
  SYS_FORK,
  SYS_EXEC,
  SYS_EXIT,
  SYS_GETPID,
  __NR_syscall
};

// System call table
extern const size_t __syscall_table[Syscall::__NR_syscall];

// Individual system call declaration.
size_t sys_uart_read(char buf[], size_t size);
size_t sys_uart_write(const char buf[], size_t size);
void sys_uart_putchar(const char c);
int sys_fork();
int sys_exec(void (*func)(), const char *const argv[]);
void sys_exit();
int sys_getpid();
/*
void sys_timer_irq_enable();
void sys_timer_irq_disable();
*/


// Indirect system call
//
// A system call is issued using the `svc 0` instruction.
// The system call number is passed via register x8,
// the parameters are stored in x0 ~ x5,
// and the return value will be stored in x0.
template <typename... Args>
size_t syscall(uint64_t id, Args ...args) {
  // Check if syscall id is valid.
  if (unlikely(id >= Syscall::__NR_syscall)) {
    printk("bad system call: (id=0x%x)\n", id);
    return -1;
  }

  /*
  auto& trap_frame = TrapFrame::get_instance();

  printk("system call: 0x%x(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
      id, args...);

  printk("system call: 0x%x(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
      trap_frame.get_x8(),
      trap_frame.get_x0(),
      trap_frame.get_x1(),
      trap_frame.get_x2(),
      trap_frame.get_x3(),
      trap_frame.get_x4(),
      trap_frame.get_x5());
      */

  return reinterpret_cast<size_t (*)(Args...)>(__syscall_table[id])(args...);
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SYSCALL_H_

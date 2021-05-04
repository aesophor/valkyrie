// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// vlibc.S - valkyrie userland C runtime library

.macro SYSCALL_DEFINE name id
.global \name
\name:
  mov x8, #\id
  svc #0
  ret
.endm


.section ".text"

SYSCALL_DEFINE read 0
SYSCALL_DEFINE write 1
SYSCALL_DEFINE open 2
SYSCALL_DEFINE close 3
SYSCALL_DEFINE uart_read 4
SYSCALL_DEFINE uart_write 5
SYSCALL_DEFINE uart_putchar 6
SYSCALL_DEFINE fork 7
SYSCALL_DEFINE exec 8
SYSCALL_DEFINE exit 9
SYSCALL_DEFINE getpid 10
SYSCALL_DEFINE wait 11
SYSCALL_DEFINE sched_yield 12
SYSCALL_DEFINE kill 13
SYSCALL_DEFINE signal 14
SYSCALL_DEFINE access 15

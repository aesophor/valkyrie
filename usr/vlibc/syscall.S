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
SYSCALL_DEFINE fork 4
SYSCALL_DEFINE exec 5
SYSCALL_DEFINE exit 6
SYSCALL_DEFINE getpid 7
SYSCALL_DEFINE wait 8
SYSCALL_DEFINE sched_yield 9
SYSCALL_DEFINE kill 10
SYSCALL_DEFINE signal 11
SYSCALL_DEFINE access 12
SYSCALL_DEFINE chdir 13
SYSCALL_DEFINE mkdir 14
SYSCALL_DEFINE rmdir 15
SYSCALL_DEFINE unlink 16
SYSCALL_DEFINE mount 17
SYSCALL_DEFINE umount 18
SYSCALL_DEFINE mknod 19
SYSCALL_DEFINE getcwd 20


// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// vlibc.S - valkyrie userland C runtime library

.section ".text"

.global __libc_start_main
__libc_start_main:
  // Initialize printf
  mov x0, #0
  ldr x1, = __libc_putchar
  bl init_printf

  // Prepare argc and argv and branch to main()
  ldp x0, x1, [sp]
  bl main

  // sys_exit()
  b  exit

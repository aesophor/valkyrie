// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);

  printf("[init] started... pid = %d\n", getpid());
  printf("argc = %d\n", argc);
  printf("argv = 0x%x\n", argv);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] (0x%x) = %s\n", i, argv[i], argv[i]);
  }

  int pid;
  int wstatus;

  switch ((pid = fork())) {
    case -1:  // error
      printf("[init] fork failed\n");
      break;

    case 0: { // child
      exec("bin/login", nullptr);
      break;
    }

    default:  // parent
      while (true) {
        pid = wait(&wstatus);
        if (pid != -1) {
          printf("[init] reaped zombie: pid = %d with error_code = %d\n", pid, wstatus);
        }
      }
      break;
  }

  return 0;
}

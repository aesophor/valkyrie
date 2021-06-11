// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

int main(int argc, char **argv) {
  int pid;
  int wstatus;

  switch ((pid = fork())) {
    case -1:  // error
      printf("init: fork failed\n");
      break;

    case 0: { // child
      char* arguments[] = {"/sbin/login", nullptr};
      exec("/sbin/login", arguments);
      break;
    }

    default:  // parent
      while (true) {
        pid = wait(&wstatus);
        if (pid != -1) {
          printf("init: reaped zombie: pid = %d with error_code = %d\n", pid, wstatus);
        }
      }
      break;
  }

  return 0;
}

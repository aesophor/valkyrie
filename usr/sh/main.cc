// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int run_shell(const char* username);

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);
  return run_shell(argv[1]);
}


int run_shell(const char* username) {
  char buf[256];

  while (true) {
    memset(buf, 0, sizeof(buf));
    printf("%s# ", username);
    uart_read(buf, 255);

    if (!strlen(buf)) {
      continue;

    } else if (!strncmp(buf, "exit", sizeof(buf))) {
      break;

    } else if (access(buf, 0) != -1) {
      int pid;
      int wstatus;

      switch ((pid = fork())) {
        case -1:
          printf("fork failed\n");
          break;

        case 0: {
          char* arguments[2] = {buf, nullptr};
          printf("exec(%s, ...)\n", buf);
          exec(buf, arguments);
          printf("exec failed\n");
          break;
        }

        default:
          wait(&wstatus);
          printf("command finished with exit code = %d\n", wstatus);
          break;
      }

    } else {
      printf("sh: command not found: %s\n", buf);
    }
  }

  return 0;
}

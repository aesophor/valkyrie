// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int run_shell(const char* username);

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);
  return run_shell(argv[1]);
}

int run_shell(const char* username) {
  char _buf[256];

  while (true) {
    memset(_buf, 0, sizeof(_buf));
    printf("%s# ", username);
    uart_read(_buf, 255);

    if (!strlen(_buf)) {
      continue;
    } else if (!strcmp(_buf, "exit")) {
      break;
    } else {
      printf("sh: command not found: %s\n", _buf);
    }
  }

  return 0;
}

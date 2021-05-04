// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);

  char username[32] = {0};
  char buf[32] = {0};

  printf("localhost login: ");
  uart_read(buf, sizeof(buf) - 1);
  strncpy(username, buf, sizeof(username) - 1);

  printf("password: ");
  uart_read(buf, sizeof(buf) - 1);

  char* arguments[4] = {"/bin/sh", username, buf, nullptr};
  exec("bin/sh", arguments);
  return 0;
}

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

bool validate_user(const char* username, const char* password);

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);

  int pid;
  char username[32];
  char password[32];

  while (true) {
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    printf("Localhost login: ");
    uart_read(username, sizeof(username) - 1);

    printf("Password: ");
    uart_read(password, sizeof(password) - 1);

    if (!validate_user(username, password)) {
      continue;
    }

    switch ((pid = fork())) {
      case -1:
        printf("fork failed\n");
        break;

      case 0: {
        char* arguments[4] = {"/bin/sh", username, nullptr};
        exec("/bin/sh", arguments);
        break;
      }

      default:
        wait(nullptr);
        break;
    }
  }

  return 0;
}


bool validate_user(const char* username, const char* password) {
  if (!strlen(username)) {
    return false;
  }

  // TODO:
  return true;
}


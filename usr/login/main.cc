// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <cstring.h>
#include <vlibc.h>

bool validate_user(const char *username, const char *password);

int main(int argc, char **argv) {
  int pid;
  char username[32];
  char password[32];

  while (true) {
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    printf("Localhost login: ");
    read(0, username, sizeof(username) - 1);

    printf("Password: ");
    read(0, password, sizeof(password) - 1);

    if (!validate_user(username, password)) {
      continue;
    }

    switch ((pid = fork())) {
      case -1:
        printf("fork failed\n");
        break;

      case 0: {
        char *arguments[4] = {"/bin/sh", username, nullptr};
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

bool validate_user(const char *username, const char *password) {
  if (!strlen(username)) {
    return false;
  }

  // TODO:
  return true;
}

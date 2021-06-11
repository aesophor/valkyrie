// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

bool validate_user(const char* username, const char* password);

int main(int argc, char **argv) {
  int pid;
  char username[32];
  char password[32];

  char msg1[] = "Localhost login: ";
  char msg2[] = "Password: ";
  char msg3[] = "fork failed\n";

  while (true) {
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    write(1, msg1, sizeof(msg1));
    read(0, username, sizeof(username) - 1);

    write(1, msg2, sizeof(msg2));
    read(0, password, sizeof(password) - 1);

    if (!validate_user(username, password)) {
      continue;
    }

    switch ((pid = fork())) {
      case -1:
        printf(msg3);
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


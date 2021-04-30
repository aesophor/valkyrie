// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int run_shell(const char* username);

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);
  return run_shell(argv[1]);
}

int get_argc(const char* s) {
  size_t len = strlen(s);
  int ret = 0;
  bool last_char_is_space = true;

  for (size_t i = 0; i < len; i++) {
    if (s[i] != ' ' && !last_char_is_space) {
      ret++;
    }
    last_char_is_space = s[i] == ' ';
  }

  return ret;
}

// Makes the current argument terminate by a NULL byte,
// and returns the pointer to the next argument.
char* get_next_arg(char* s) {
  char* ptr = s;
  while (*ptr && *ptr != ' ') ptr++;
  *ptr++ = 0;
  while (*ptr && *ptr == ' ') ptr++;
  return ptr;
}

int run_shell(const char* username) {
  char buf[256];
  int wstatus = 0;
  int argc;
  char* str;
  char prompt = (!strncmp(username, "root", 4)) ? '#' : '$';

  while (true) {
    memset(buf, 0, sizeof(buf));
    printf("[%s@localhost]%c ", username, prompt);
    uart_read(buf, 255);

    argc = get_argc(buf);
    str = buf;

    char* arguments[argc + 1];
    arguments[0] = str;
    for (int i = 1; i < argc; i++) {
      str = get_next_arg(str);
      arguments[i] = str;
    }
    arguments[argc] = nullptr;

    if (!strlen(buf)) {
      continue;

    } else if (!strncmp(buf, "exit", sizeof(buf))) {
      break;

    } else if (!strncmp(buf, "$?", sizeof(buf))) {
      printf("%d\n", wstatus);

    } else if (access(arguments[0], 0) != -1) {
      int pid;
      int wstatus;

      switch ((pid = fork())) {
        case -1:
          printf("fork failed\n");
          break;

        case 0: {
          exec(arguments[0], arguments);

          printf("exec failed\n");
          exit(-1);
          break;
        }

        default:
          wait(&wstatus);
          break;
      }

    } else {
      printf("sh: command not found: %s\n", buf);
    }
  }

  return 0;
}

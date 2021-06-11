// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int run_shell(const char* username);

int main(int argc, char **argv) {
  return run_shell(argv[1]);
}

int get_argc(const char* s) {
  size_t len = strlen(s);
  int ret = 0;

  size_t begin = 0;
  for (size_t i = 0; i < len; i++) {
    char c = s[i];
    if (c == ' ') {
      size_t len = i - begin;
      if (len) {
        ret++;
      }
      begin = i + 1;
    }
  }

  size_t tail_len = len - begin;
  if (tail_len) {
    ret++;
  }
  return ret;
}

// Makes the current argument terminate by a NULL byte,
// and returns the pointer to the next argument.
char* get_next_arg(char* s) {
  while (*s && *s != ' ') s++;
  *s++ = 0;
  while (*s && *s == ' ') s++;
  return s;
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
    read(0, buf, 255);

    argc = get_argc(buf);
    str = buf;
    while (*str && *str == ' ' && str < buf + sizeof(buf)) str++;

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

    } else if (!strncmp(buf, "cd", sizeof(buf))) {
      if (chdir(arguments[1]) == -1) {
        printf("cd: no such file or directory: %s\n", arguments[1]);
      }

    } else if (!strncmp(buf, "echo $?", sizeof(buf))) {
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
      printf("sh: command not found: %s\n", arguments[0]);
    }
  }

  return 0;
}

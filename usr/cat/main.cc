// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int main(int argc, char **argv) {
  int fd;
  int ret;
  char buf[512] = {0};

  init_printf(nullptr, __libc_putchar);

  if (argc < 1) {
    printf("cat: argv[0] should be the path to cat\n");
    ret = -1;
    goto out;
  }

  if (argc < 2) {
    printf("usage: %s <file>\n", argv[0]);
    ret = 0;
    goto out;
  }

  if ((fd = open(argv[1], 0)) == -1) {
    printf("ls: cannot access '%s': No such file or directory\n", argv[1]);
    ret = 2;
    goto out;
  }

  if ((ret = read(fd, buf, sizeof(buf))) != -1) {
    write(1, buf, ret);
    ret = 0;
  } else {
    printf("%s: %s: Permission denied\n", argv[0], argv[1]);
  }

out:
  return ret;
}

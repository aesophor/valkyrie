// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

#define O_CREAT ((1 << 3))

int main(int argc, char **argv) {
  int fd;
  int ret;

  if (argc < 2) {
    printf("%s: missing file operand\n", argv[0]);
    ret = 1;
    goto out;
  }

  // If file doesnt exist, then create it with open()
  if (access(argv[1], 0) == -1) {
    fd = open(argv[1], O_CREAT);
    close(fd);
  }

out:
  return ret;
}

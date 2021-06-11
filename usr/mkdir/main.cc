// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int main(int argc, char **argv) {
  int ret;

  if (argc < 2) {
    printf("mkdir: missing operand\n");
    ret = 0;
    goto out;
  }

  if ((ret = mkdir(argv[1])) == -1) {
    printf("mkdir: cannot create '%s'\n", argv[1]);
    ret = 1;
    goto out;
  }

out:
  return ret;
}

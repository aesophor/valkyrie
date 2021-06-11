// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

#define DENTRY_NAME_LEN 256

struct DirectoryEntry {
  char name[DENTRY_NAME_LEN];
};

int main(int argc, char **argv) {
  int fd;
  int ret;
  DirectoryEntry dentry;

  if (argc < 1) {
    printf("ls: argv[0] should be the path to ls\n");
    ret = -1;
    goto out;
  }

  if (argc == 1) {
    argv[1] = ".";
  }

  if ((fd = open(argv[1], 0)) == -1) {
    printf("ls: cannot access '%s': No such file or directory\n", argv[1]);
    ret = 2;
    goto out;
  }

  while ((ret = read(fd, reinterpret_cast<char*>(&dentry), sizeof(dentry)))) {
    printf("%s ", dentry.name);
  }
  printf("\n");

out:
  return ret;
}

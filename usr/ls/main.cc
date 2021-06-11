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
    char msg[] = "ls: argv[0] should be the path to ls\n";
    write(1, msg, sizeof(msg) - 1);
    ret = -1;
    goto out;
  }

  if (argc == 1) {
    argv[1] = ".";
  }

  if ((fd = open(argv[1], 0)) == -1) {
    char msg1[] = "ls: cannot access '";
    char msg2[] = "': No such file or directory\n";
    write(1, msg1, sizeof(msg1) - 1);
    write(1, argv[1], strlen(argv[1]));
    write(1, msg2, sizeof(msg2) - 1);

    ret = 2;
    goto out;
  }

  while ((ret = read(fd, reinterpret_cast<char*>(&dentry), sizeof(dentry)))) {
    write(1, dentry.name, strlen(dentry.name));
    write(1, " ", 1);
  }
  write(1, "\n", 1);

out:
  return ret;
}

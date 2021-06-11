// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int main(int argc, char **argv) {
  char buf[8];
  mkdir("mnt");
  int fd = open("/mnt/a.txt", O_CREAT);
  write(fd, "Hi", 2);
  close(fd);
  chdir("mnt");
  fd = open("a.txt", 0);
  assert(fd >= 0);
  read(fd, buf, 2);
  assert(strncmp(buf, "Hi", 2) == 0);

  chdir("..");
  mount("tmpfs", "mnt", "tmpfs");
  fd = open("mnt/a.txt", 0);
  assert(fd < 0);

  umount("/mnt");
  fd = open("/mnt/a.txt", 0);
  assert(fd >= 0);
  read(fd, buf, 2);
  assert(strncmp(buf, "Hi", 2) == 0);

  printf("vfs_test_mnt passed successfully\n");
  return 0;
}

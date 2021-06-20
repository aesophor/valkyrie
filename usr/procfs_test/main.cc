// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

int main() {
  char buf[16];
  mkdir("proc");
  mount("procfs", "proc", "procfs");
  int fd = open("/proc/switch", 0);
  write(fd, "0", 1);
  close(fd);

  fd = open("/proc/hello", 0);
  int sz = read(fd, buf, 16);
  buf[sz] = '\0';
  printf("%s\n", buf);  // should be hello
  close(fd);

  fd = open("/proc/switch", 0);
  write(fd, "1", 1);
  close(fd);

  fd = open("/proc/hello", 0);
  sz = read(fd, buf, 16);
  buf[sz] = '\0';
  printf("%s\n", buf);  //should be HELLO
  close(fd);

  fd = open("/proc/1/status", 0);  // choose a created process's id here
  sz = read(fd, buf, 16);
  buf[sz] = '\0';
  printf("%s\n", buf);  // process's status.
  close(fd);

  fd = open("/proc/999/status", 0);  // choose a non-existed process's id here
  assert(fd < 0);
}

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

#define O_CREAT ((1 << 3))

int main(int argc, char **argv) {
  char s[] = "this message is written to /dev/console, "
             "now type something: ";

  char buf[32];

  memset(buf, 0, sizeof(buf));

  int fd = open("/dev/console", 0);
  write(fd, s, sizeof(s) - 1);
  read(fd, buf, sizeof(buf) - 1);

  printf("received via /dev/console: %s\n", buf);
  return 0;
}

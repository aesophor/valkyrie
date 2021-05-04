// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

#define O_CREAT ((1 << 3))

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);

  char buf[32];
  for (int i = 0; i < 32; i++) {
    buf[i] = 0;
  }

  int a = open("hello", O_CREAT);
  int b = open("world", O_CREAT);
  printf("a = %d, b = %d\n", a, b);
  write(a, "Hello ", 6);
  write(b, "World!", 6);
  close(a);
  close(b);
  b = open("hello", 0);
  a = open("world", 0);
  printf("b = %d, a = %d\n", b, a);
  int sz;
  sz = read(b, buf, 100);
  sz += read(a, buf + sz, 100);
  buf[sz] = '\0';
  printf("%s\n", buf); // should be Hello World!

  return 0;
}

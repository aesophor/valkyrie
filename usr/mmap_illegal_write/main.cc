// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

int main(int argc, char **argv) {
  char *ptr = (char *) mmap(nullptr, 4096, PROT_READ, MAP_ANONYMOUS, -1, 0);
  printf("addr: 0x%p\n", ptr);
  printf("%d\n", ptr[1000]);  // should be 0
  ptr[0] = 1;                 // should be seg fault
  printf("%d\n", ptr[0]);     // not reached
  return 0;
}

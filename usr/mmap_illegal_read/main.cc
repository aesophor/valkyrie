// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

int main(int argc, char **argv) {
  void *addr = reinterpret_cast<void *>(0x1000);
  char *ptr = (char *) mmap(addr, 4096, PROT_READ, MAP_ANONYMOUS, -1, 0);
  printf("addr: 0x%p\n", ptr);
  printf("%d\n", ptr[1000]);  // should be 0
  printf("%d\n", ptr[4097]);  // should be segfault
  return 0;
}

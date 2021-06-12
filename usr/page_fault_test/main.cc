// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

int main(int argc, char **argv) {
  int cnt = 0;
  if(fork() == 0) {
    fork();
    fork();
    while(cnt < 10) {
      printf("pid: %d, sp: 0x%llx cnt: %d\n", getpid(), &cnt, cnt++); // address should be the same, but the cnt should be increased indepndently
    }
  } else {
    int* a = 0x0; // a non-mapped address.
    printf("%d\n", *a); // trigger simple page fault.
    printf("Should not be printed\n");
  }
}

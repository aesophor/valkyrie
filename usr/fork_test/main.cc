// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

void fork_test() {
  printf("Fork Test, pid %d\n", getpid());
  size_t cnt = 1;
  size_t ret = 0;

  int wstatus;
  int pid;

  if ((ret = fork()) == 0) {  // child
    printf("pid: %d, cnt: %d, ptr: 0x%p\n", getpid(), cnt, &cnt);
    ++cnt;
    fork();
    while (cnt < 5) {
      printf("pid: %d, cnt: %d, ptr: 0x%p\n", getpid(), cnt, &cnt);
      ++cnt;
    }
    printf("child terminating...\n");
  } else {
    printf("parent here, pid %d, child %d\n", getpid(), ret);

    pid = wait(&wstatus);
    printf("waited child with pid = %d and error_code = %d\n", pid, wstatus);
    printf("parent terminating...\n");
  }
  asm volatile("mov %0, sp" : "=r"(cnt));
  asm volatile("mov %0, lr" : "=r"(ret));
  printf("SP = 0x%p, LR = 0x%p\n", cnt, ret);
}

int main(int argc, char **argv) {
  printf("argc = %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  fork_test();
  return 0;
}

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>

void on_sigsegv(int sig) {
  printf("user's sigsegv handler \\|/\n");
  exit(1);
}

int main(int argc, char **argv) {
  signal(SIGSEGV, on_sigsegv);

  int *p = nullptr;
  *p = 3;

  return 0;
}

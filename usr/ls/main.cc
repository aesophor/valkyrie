// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <vlibc.h>
#include <cstring.h>

int main(int argc, char **argv) {
  init_printf(nullptr, __libc_putchar);



  return 0;
}

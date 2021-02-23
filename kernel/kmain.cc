// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.

extern "C" void kmain(char* bss_start, char* bss_end) {
  // Initialize .bss section to 0
  char *ptr = bss_start;
  while (ptr <= bss_end) {
    *ptr = 0x00;
    ++ptr;
  }

  while (1);
}

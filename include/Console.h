// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONSOLE_H_
#define VALKYRIE_CONSOLE_H_

#include <Types.h>
#include <MiniUART.h>
#include <libs/printf.h>

namespace valkyrie::kernel::console {

void initialize(MiniUART* mini_uart);

}  // namespace valkyrie::kernel::console


#define printf  tfp_printf
#define sprintf tfp_sprintf

// FIXME: not sure why we cannot access cntpct_el0 at EL0...(?)
template <typename... Args>
void printk(char* fmt, Args&& ...args) {
  uint64_t cntpct_el0;
  uint64_t cntfrq_el0;
  uint64_t timestamp;

  asm volatile("mrs %0, cntpct_el0" : "=r" (cntpct_el0));
  asm volatile("mrs %0, cntfrq_el0" : "=r" (cntfrq_el0));
  timestamp = 1000 * cntpct_el0 / cntfrq_el0;

  printf("[%d.%d] ", timestamp / 1000, timestamp % 1000);
  printf(fmt, args...);
}


extern "C" {

char _recv();
char getchar();
void putchar(const char c);
void gets(char* s);
void puts(const char* s, bool newline = true);

}

#endif  // VALKYRIE_CONSOLE_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONSOLE_H_
#define VALKYRIE_CONSOLE_H_

#include <Types.h>
#include <dev/ConsoleColors.h>
#include <driver/MiniUART.h>
#include <libs/printf.h>

namespace valkyrie::kernel::console {

void initialize(MiniUART* mini_uart);

void set_color(Color fg_color, bool bold = false);
void clear_color();

}  // namespace valkyrie::kernel::console


template <typename... Args>
void printf(const char* fmt, Args&&... args) {
  tfp_printf(const_cast<char*>(fmt), args...);
}

template <typename... Args>
void sprintf(char* s, const char* fmt, Args&&... args) {
  tfp_sprintf(s, const_cast<char*>(fmt), args...);
}

template <typename... Args>
void printk(const char* fmt, Args&&... args) {
  // FIXME: not sure why we cannot access cntpct_el0 at EL0...(?)
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

char getchar();
void gets(char* s);
void putchar(const char c);
void puts(const char* s, bool newline = true);

}

#endif  // VALKYRIE_CONSOLE_H_

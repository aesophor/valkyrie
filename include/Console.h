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

extern "C" uint32_t get_cntfrq_el0(void);
extern "C" uint32_t get_cntpct_el0(void);

template <typename... Args>
void printk(char* fmt, Args&& ...args) {
  uint32_t timestamp = 1000 * get_cntpct_el0() / get_cntfrq_el0();
  printf("[%d.%06d] ", timestamp / 1000, timestamp % 1000);
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

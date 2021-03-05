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

extern "C" {

char getchar();
void putchar(const char c);
void gets(char* s);
void puts(const char* s, bool newline = true);

}

#endif  // VALKYRIE_CONSOLE_H_

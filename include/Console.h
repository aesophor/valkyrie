// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONSOLE_H_
#define VALKYRIE_CONSOLE_H_

#include <Types.h>
#include <MiniUART.h>

namespace valkyrie::kernel {

void console_init(MiniUART* mini_uart);

char getchar();
void putchar(const char c);
void gets(char* s);
void puts(const char* s, bool newline = true);
void print_hex(const uint32_t value);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CONSOLE_H_

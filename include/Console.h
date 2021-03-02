// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONSOLE_H_
#define VALKYRIE_CONSOLE_H_

#include <Types.h>
#include <MiniUART.h>

namespace valkyrie::kernel::console {

void initialize(MiniUART* mini_uart);

}  // namespace valkyrie::kernel::console


extern "C" {

char getchar();
void putchar(const char c);
void gets(char* s);
void puts(const char* s, bool newline = true);
void print_hex(const uint32_t value);

}

#endif  // VALKYRIE_CONSOLE_H_

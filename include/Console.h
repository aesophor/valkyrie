// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONSOLE_H_
#define VALKYRIE_CONSOLE_H_

#include <Types.h>
#include <MiniUART.h>

namespace {

valkyrie::kernel::MiniUART* mini_uart;

}  // namespace


namespace valkyrie::kernel {

void console_init(MiniUART* mini_uart);

void gets(char* s);
void puts(const char* s);
void putchar(const char c);
void print_hex(const uint32_t value);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CONSOLE_H_

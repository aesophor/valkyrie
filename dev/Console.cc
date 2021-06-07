// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/Console.h>

#include <driver/MiniUART.h>

namespace valkyrie::kernel {

Console& Console::get_instance() {
  static Console instance(MiniUART::get_instance());
  return instance;
}

Console::Console(CharacterDevice::Driver& driver)
    : CharacterDevice("console", driver) {
  auto __putchar = [](void*, char c) { putchar(c); };
  init_printf(nullptr, __putchar);
}


char Console::read_char() {
  return _driver.read_char();
}

void Console::write_char(const char c) {
  _driver.write_char(c);
}

int Console::read(char buf[], size_t size) {
  int i = 0;
  while (i < (int) size) {
    auto c = read_char();

    if (c == 0x7f) {
      if (i > 0) {
        buf[--i] = 0;
        write_char('\b');
        write_char(' ');
        write_char('\b');
      }
    } else if (c == '\n') {
      return i;
    } else {
      buf[i++] = c;
    }
  }

  return size;
}

int Console::write(const char buf[], size_t size) {
  for (size_t i = 0; i < size; i++) {
    write_char(buf[i]);
  }
  return size;
}


void Console::set_color(Console::Color fg_color, bool bold) {
  printf("\033[%d;3%dm", bold, fg_color);
}

void Console::clear_color() {
  static const char s[] = "\033[0m";
  write(s, sizeof(s));
}

}  // namespace valkyrie::kernel::console


using valkyrie::kernel::Console;

extern "C" {
char getchar() { return Console::get_instance().read_char(); }
void putchar(const char c) { Console::get_instance().write_char(c); }
}

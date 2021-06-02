// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONSOLE_H_
#define VALKYRIE_CONSOLE_H_

#include <Types.h>
#include <dev/CharacterDevice.h>
#include <libs/printf.h>

namespace valkyrie::kernel {

class Console : public CharacterDevice {
 public:
  enum class Color {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE
  };

  static Console& get_instance();

  virtual ~Console() = default;
  Console(const Console&) = delete;
  Console(Console&&) = delete;
  Console& operator =(const Console&) = delete;
  Console& operator =(Console&&) = delete;

  virtual char read_char() override;
  virtual void write_char(const char c) override; 

  int read(char buf[], size_t size);
  int write(const char buf[], size_t size);


  void set_color(Console::Color fg_color, bool bold = false);
  void clear_color();

 protected:
  Console(CharacterDevice::Driver& driver);
};

}  // namespace valkyrie::kernel


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
void putchar(const char c);
}

#endif  // VALKYRIE_CONSOLE_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MINI_UART_H_
#define VALKYRIE_MINI_UART_H_

#include <Singleton.h>
#include <Types.h>

#include <dev/CharacterDevice.h>
#include <driver/GPIO.h>

namespace valkyrie::kernel {

class MiniUART : public Singleton<MiniUART>, public CharacterDevice::Driver {
 public:
  virtual ~MiniUART() = default;

  virtual char read_char() override {
    return getchar();
  }
  virtual void write_char(const char c) override {
    putchar(c);
  }

  char getchar() {
    return getchar_sync();
  }
  void gets(char *s) {
    gets_sync(s);
  }
  void putchar(const char c) {
    putchar_sync(c);
  }
  void puts(const char *s, bool newline = true) {
    puts_sync(s, newline);
  }

 protected:
  MiniUART();

 private:
  uint8_t recv();
  void send(const uint8_t byte);

  // Synchronous I/O
  char getchar_sync();
  void gets_sync(char *s);
  void putchar_sync(const char c);
  void puts_sync(const char *s, bool newline = true);
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MINI_UART_H_

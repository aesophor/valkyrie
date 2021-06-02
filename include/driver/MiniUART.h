// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MINI_UART_H_
#define VALKYRIE_MINI_UART_H_

#include <Types.h>
#include <dev/CharacterDevice.h>
#include <driver/GPIO.h>

#define READ_BUFFER_SIZE  512
#define WRITE_BUFFER_SIZE 512

namespace valkyrie::kernel {

class MiniUART : public CharacterDevice::Driver {
 public:
  static MiniUART& get_instance();

  ~MiniUART() = default;
  MiniUART(const MiniUART&) = delete;
  MiniUART(MiniUART&&) = delete;
  MiniUART& operator =(const MiniUART&) = delete;
  MiniUART& operator =(MiniUART&&) = delete;

  virtual char read_char() override;
  virtual void write_char(const char c) override;

  char getchar();
  void gets(char* s);
  void putchar(const char c);
  void puts(const char* s, bool newline = true);

  void enable_interrupts() const;
  void disable_interrupts() const;
  bool has_pending_irq() const;
  void handle_irq();

  bool is_debugging() const;
  void set_debugging(bool debugging);
  void set_read_buffer_enabled(bool enabled);
  void set_write_buffer_enabled(bool enabled);

 private:
  MiniUART();

  // Synchronous I/O
  uint8_t recv();
  void send(const uint8_t byte);
  char getchar_sync();
  void gets_sync(char* s);
  void putchar_sync(const char c);
  void puts_sync(const char* s, bool newline = true);

  // Asynchronous I/O
  void handle_tx_irq();
  void handle_rx_irq();
  void flush_write_buffer();
  char getchar_async();
  void gets_async(char* s);
  void putchar_async(const char c);
  void puts_async(const char* s, bool newline = true);


  bool _is_debugging;
  bool _is_read_buffer_enabled;
  bool _is_write_buffer_enabled;

  int _read_buffer_bytes_pending;
  int _write_buffer_bytes_pending;

  uint8_t _read_buffer[READ_BUFFER_SIZE];
  uint8_t _write_buffer[WRITE_BUFFER_SIZE];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MINI_UART_H_

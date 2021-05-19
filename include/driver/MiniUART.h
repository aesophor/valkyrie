// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MINI_UART_H_
#define VALKYRIE_MINI_UART_H_

#include <Types.h>
#include <driver/GPIO.h>

// Quoting from BCM2837-ARM-Peripherals.pdf (pg. 6)
// ------------------------------------------------
// Physical addresses range from 0x3F000000 to 0x3FFFFFFF for peripherals.
// The bus addresses for peripherals are set up to map onto the peripheral
// bus address range starting at 0x7E000000. Thus a peripheral advertised here
// at bus address 0x7Ennnnnn is available at physical address 0x3Fnnnnnn.
#define AUX_ENABLES     (MMIO_BASE + 0x215004)
#define AUX_MU_IO_REG   (MMIO_BASE + 0x215040)
#define AUX_MU_IER_REG  (MMIO_BASE + 0x215044)
#define AUX_MU_IIR_REG  (MMIO_BASE + 0x215048)
#define AUX_MU_LCR_REG  (MMIO_BASE + 0x21504c)
#define AUX_MU_MCR_REG  (MMIO_BASE + 0x215050)
#define AUX_MU_LSR_REG  (MMIO_BASE + 0x215054)
#define AUX_MU_MSR_REG  (MMIO_BASE + 0x215058)
#define AUX_MU_SCRATCH  (MMIO_BASE + 0x21505c)
#define AUX_MU_CNTL_REG (MMIO_BASE + 0x215060)
#define AUX_MU_STAT_REG (MMIO_BASE + 0x215064)
#define AUX_MU_BAUD_REG (MMIO_BASE + 0x215068)

#define GPFSEL_INPUT  0b000
#define GPFSEL_OUTPUT 0b001
#define GPFSEL_ALT0   0b100
#define GPFSEL_ALT1   0b101
#define GPFSEL_ALT2   0b110
#define GPFSEL_ALT3   0b111
#define GPFSEL_ALT4   0b011
#define GPFSEL_ALT5   0b010

#define READ_BUFFER_SIZE  512
#define WRITE_BUFFER_SIZE 512

namespace valkyrie::kernel {

class MiniUART {
 public:
  static MiniUART& get_instance();

  ~MiniUART() = default;
  MiniUART(const MiniUART&) = delete;
  MiniUART(MiniUART&&) = delete;
  MiniUART& operator =(const MiniUART&) = delete;
  MiniUART& operator =(MiniUART&&) = delete;

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

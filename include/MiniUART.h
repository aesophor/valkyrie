// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MINI_UART_H_
#define VALKYRIE_MINI_UART_H_

#include <GPIO.h>
#include <Types.h>

// Quoting from BCM2837-ARM-Peripherals.pdf (pg. 6)
// ------------------------------------------------
// Physical addresses range from 0x3F000000 to 0x3FFFFFFF for peripherals.
// The bus addresses for peripherals are set up to map onto the peripheral
// bus address range starting at 0x7E000000. Thus a peripheral advertised here
// at bus address 0x7Ennnnnn is available at physical address 0x3Fnnnnnn.
#define AUX_ENABLES     (MMIO_BASE + 0X215004)
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

namespace valkyrie::kernel {

class MiniUART {
 public:
  MiniUART();
  ~MiniUART() = default;

  char getchar();
  void putchar(const char c);

  void gets(char* s);
  void puts(const char* s);

 private:
  uint8_t recv();
  void send(const uint8_t byte);
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MINI_UART_H_

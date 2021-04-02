// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// [PART 1] Configure General-Purpose I/O Function Selector
//
// According to: https://grasslab.github.io/NYCU_Operating_System_Capstone/hardware/uart.html#gpio
// To use mini UART, we should set both gpio14 and gpio15 to use ALT5.
//
// In order to do so, we need to find the correct GPFSEL register (function selector),
// and in our case it is GPFSEL1 because it manages gpio10 ~ gpio19.
// The GPFSEL1 register manages 9 gpio pins, where each pin is represented by
// 3 bits and with different combinations of those 3 bits that gpio pin
// will use different alternative functions.
//
//
// [PART 2] Configure pull up/down register to disable GPIO pull up/down
//
// 1. Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down
//    or neither to remove the current Pull-up/down)
// 2. Wait 150 cycles – this provides the required set-up time for the control signal
// 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
//    modify – NOTE only the pads which receive a clock will be modified, all others
//    will retain their previous state.
// 4. Wait 150 cycles – this provides the required hold time for the control signal
// 5. Write to GPPUD to remove the control signal
// 6. Write to GPPUDCLK0/1 to remove the clock
//
//
// [PART 3] Finally, enable mini UART.
//
// 1. Set AUXENB register to enable mini UART,
//    and then mini UART register can be accessed.
// 2. Set AUX_MU_CNTL_REG to 0,
//    which disables transmitter and receiver during configuration.
// 3. Set AUX_MU_IER_REG to 0,
//    which disables interrupts because currently you don’t need interrupt.
// 4. Set AUX_MU_LCR_REG to 3,
//    which sets the data size to 8 bit.
// 5. Set AUX_MU_MCR_REG to 0,
//    because we don’t need auto flow control.
// 6. Set AUX_MU_BAUD to 270,
//    which sets baud rate to 115200
// 7. Set AUX_MU_IIR_REG to 6. No FIFO.
// 8. Set AUX_MU_CNTL_REG to 3. Enable the transmitter and receiver.
// ------------
// Reference:
// [1] https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf
// [2] https://s-matyukevich.github.io/raspberry-pi-os/docs/lesson01/rpi-os.html
#include <dev/MiniUART.h>

#include <dev/Console.h>
#include <dev/IO.h>
#include <libs/String.h>

namespace valkyrie::kernel {

MiniUART::MiniUART() {
  // Configure GPFSEL1 register to set both gpio14 and gpio15 to use ALT5.
  uint32_t reg = io::get<uint32_t>(GPFSEL1);
  reg &= ~(0b111 << 12);     // clear the 12~15th bits (gpio14)
  reg |= GPFSEL_ALT5 << 12;  // set ALT5 to 12~15th bits (gpio14)
  reg &= ~(0b111 << 15);     // clear the 15~18th bits (gpio15)
  reg |= GPFSEL_ALT5 << 15;  // set ALT5 to 15~18th bits (gpio15)
  io::put<uint32_t>(GPFSEL1, reg);

  // Disable GPIO PUD (Pull-Up/Down) by configuring PUD register.
  io::put<uint32_t>(GPPUD, PUD_DISABLED);               // disable PUD
  io::delay(150);                                       // `nop` 150 times
  io::put<uint32_t>(GPPUDCLK0, (1 << 14) | (1 << 15));  // set GPPUDCLK0
  io::delay(150);                                       // `nop` 150 times
  io::put<uint32_t>(GPPUD, PUD_DISABLED);               // remove the control signal
  io::put<uint32_t>(GPPUDCLK0, 0);                      // remove the clock

  // Enable mini UART.
  io::put<uint32_t>(AUX_ENABLES, 1);        // enable mini UART and access to mini UART registers
  io::put<uint32_t>(AUX_MU_CNTL_REG, 0);    // disable tx and rx during configuration
  io::put<uint32_t>(AUX_MU_IER_REG, 0);     // disable interrupts
  io::put<uint32_t>(AUX_MU_LCR_REG, 3);     // sets the data size to 8 bit
  io::put<uint32_t>(AUX_MU_MCR_REG, 0);     // disable auto flow control
  io::put<uint32_t>(AUX_MU_BAUD_REG, 270);  // set baud rate to 115200
  io::put<uint32_t>(AUX_MU_IIR_REG, 6);     // no FIFO
  io::put<uint32_t>(AUX_MU_CNTL_REG, 3);    // re-enable tx and rx

  // Register MiniUART as the driver for console.
  console::initialize(this);
}


uint8_t MiniUART::recv() {
  while (!(io::get<uint32_t>(AUX_MU_LSR_REG) & 1));
  return io::get<uint8_t>(AUX_MU_IO_REG);
}

void MiniUART::send(const uint8_t byte) {
  while (!(io::get<uint32_t>(AUX_MU_LSR_REG) & 0x20));
  io::put<uint8_t>(AUX_MU_IO_REG, byte);
}


char MiniUART::getchar() {
  char c = recv();
  c = (c == '\r') ? '\n' : c;
  putchar(c);
  return c;
}

void MiniUART::putchar(const char c) {
  if (c == '\n') {
    send('\r');
    send('\n');
  } else {
    send(c);
  }
}


void MiniUART::gets(char* s) {
  const char* const begin = s;
  char c;

  while ((c = getchar()) != '\n') {
    if (c == 0x7f) {  // is backspace
      if (s > begin) {
        *s-- = 0;
        putchar('\b');
        putchar(' ');
        putchar('\b');
      }
    } else {
      *s++ = c;
    }
  }

  *s = 0;
}

void MiniUART::puts(const char* s, bool newline) {
  for (size_t i = 0; i < strlen(s); i++) {
    putchar(s[i]);
  }
  if (newline) {
    putchar('\n');
  }
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// According to: https://grasslab.github.io/NYCU_Operating_System_Capstone/hardware/uart.html#gpio
// To use mini UART, we should set both gpio14 and gpio15 to use ALT5.
//
// In order to do so, we need to find the correct GPFSEL register (function selector),
// and in our case it is GPFSEL1 because it manages gpio10 ~ gpio19.
//
// The GPFSEL1 register manages 9 gpio pins, where each pin is represented by
// 3 bits and with different combinations of those 3 bits that gpio pin
// will use different alternative functions:
//
// 000 - input
// 001 - output
// 100 - ALT0
// 101 - ALT1
// 110 - ALT2
// 111 - ALT3
// 011 - ALT4
// 010 - ALT5
//
// ------------
// Reference:
// [1] https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf
// [2] https://s-matyukevich.github.io/raspberry-pi-os/docs/lesson01/rpi-os.html

#include <mini_uart.h>

namespace valkyrie {

mini_uart::mini_uart() {
  // configure GPFSELn register to change alternate function.
  uint32_t reg = READ32(GPFSEL1);
  reg &= ~(0b111 << 12);
  reg |= ALT5 << 12;      
  reg &= ~(0b111 << 15);
  reg |= ALT5 << 15;
  WRITE32(GPFSEL1, reg);

  // Configure pull up/down register to disable GPIO pull up/down
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
  WRITE32(GPPUD, PUD_DISABLED);
  DELAY(150);
  WRITE32(GPPUDCLK0, (1 << 14) | (1 << 15));
  DELAY(150);
  WRITE32(GPPUDCLK1, 0);

  // Finally, enable mini UART.
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
  WRITE32(AUX_ENABLES, 1);
  WRITE32(AUX_MU_CNTL_REG, 0);
  WRITE32(AUX_MU_IER_REG, 0);
  WRITE32(AUX_MU_LCR_REG, 3);
  WRITE32(AUX_MU_MCR_REG, 0);
  WRITE32(AUX_MU_BAUD_REG, 270);
  WRITE32(AUX_MU_IIR_REG, 6);
  WRITE32(AUX_MU_CNTL_REG, 3);
}


char mini_uart::read() {
  while (!(READ32(AUX_MU_LSR_REG) & 1));
  return READ32(AUX_MU_IO_REG & 0xff);
}

void mini_uart::write(const char c) {
  while (!(READ32(AUX_MU_LSR_REG) & 0b100000));
  WRITE32(AUX_MU_IO_REG, c);
}

}  // namespace valkyrie

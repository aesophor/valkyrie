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
#include <libs/CString.h>

#define BACKSPACE 0x7f

namespace valkyrie::kernel {

MiniUART& MiniUART::get_instance() {
  static MiniUART instance;
  return instance;
}

MiniUART::MiniUART()
    : _is_buffer_enabled(),
      _read_buffer_bytes_pending(),
      _write_buffer_bytes_pending(),
      _read_buffer(),
      _write_buffer() {
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
  io::put<uint32_t>(AUX_MU_CNTL_REG, 0);    // disable tx/rx during configuration
  io::put<uint32_t>(AUX_MU_IER_REG, 0b11);  // enable tx/rx interrupts
  io::put<uint32_t>(AUX_MU_LCR_REG, 3);     // sets the data size to 8 bit
  io::put<uint32_t>(AUX_MU_MCR_REG, 0);     // disable auto flow control
  io::put<uint32_t>(AUX_MU_BAUD_REG, 270);  // set baud rate to 115200
  io::put<uint32_t>(AUX_MU_IIR_REG, 1);     // FIFO empty, currently no irq pending
  io::put<uint32_t>(AUX_MU_CNTL_REG, 3);    // re-enable tx/rx

  // Register MiniUART as the driver for console.
  console::initialize(this);
}



char MiniUART::getchar() {
  return (_is_buffer_enabled) ? getchar_async() : getchar_sync();
}

void MiniUART::gets(char* s) {
  (_is_buffer_enabled) ? gets_async(s) : gets_sync(s);
}

void MiniUART::putchar(const char c) {
  (_is_buffer_enabled) ? putchar_async(c) : putchar_sync(c);
}

void MiniUART::puts(const char* s, bool newline) {
  (_is_buffer_enabled) ? puts_async(s, newline) : puts_sync(s, newline);
}



uint8_t MiniUART::recv() {
  while (!(io::get<uint32_t>(AUX_MU_LSR_REG) & 1));
  return io::get<uint8_t>(AUX_MU_IO_REG);
}

void MiniUART::send(const uint8_t byte) {
  while (!(io::get<uint32_t>(AUX_MU_LSR_REG) & 0x20));
  io::put<uint8_t>(AUX_MU_IO_REG, byte);
}

char MiniUART::getchar_sync() {
  char c = recv();
  c = (c == '\r') ? '\n' : c;
  putchar_sync(c);
  return c;
}

void MiniUART::gets_sync(char* s) {
  const char* const begin = s;
  char c;

  while ((c = getchar_sync()) != '\n') {
    if (c == BACKSPACE) {
      if (s > begin) {
        *s-- = 0;
        putchar_sync('\b');
        putchar_sync(' ');
        putchar_sync('\b');
      }
    } else {
      *s++ = c;
    }
  }
  *s = 0;
}

void MiniUART::putchar_sync(const char c) {
  if (c == '\n') {
    send('\r');
    send('\n');
  } else {
    send(c);
  }
}

void MiniUART::puts_sync(const char* s, bool newline) {
  for (size_t i = 0; i < strlen(s); i++) {
    putchar_sync(s[i]);
  }
  if (newline) {
    putchar_sync('\n');
  }
}



void MiniUART::enable_interrupts() const {
  // Enable mini UART interrupt routing
  io::put<uint32_t>(0x3f00b210, 1 << 29);
}

void MiniUART::disable_interrupts() const {
  // Disable mini UART interrupt routing
  io::put<uint32_t>(0x3f00b21c, 1 << 29);
}

void MiniUART::handle_rx_irq() {
  auto byte = io::get<uint8_t>(AUX_MU_IO_REG);

  if (byte == BACKSPACE) {
    if (_read_buffer_bytes_pending > 0) {
      _read_buffer_bytes_pending--;
    }
  } else {
    byte = (byte == '\r') ? '\n' : byte;
    _read_buffer[_read_buffer_bytes_pending++] = byte;
  }

  putchar_async(byte);

  /*
  printf("[");
  for (int i = 0; i < _read_buffer_bytes_pending; i++) {
    printf("0x%x,", _read_buffer[i]);
  }
  printf("] (%d)\n", _read_buffer_bytes_pending);
  */
}

void MiniUART::handle_tx_irq() {
  flush_write_buffer();
}

void MiniUART::flush_write_buffer() {
  for (int i = 0; i < _write_buffer_bytes_pending; i++) {
    putchar_sync(_write_buffer[i]);
  }
  _write_buffer_bytes_pending = 0;
}

char MiniUART::getchar_async() {
  enable_interrupts();

  while (!_read_buffer_bytes_pending) {
    enable_interrupts();
  }

  char c = _read_buffer[0];
  _read_buffer_bytes_pending--;
  return 0;
}

void MiniUART::gets_async(char* s) {
  enable_interrupts();

  while (_read_buffer_bytes_pending == 0) {
    enable_interrupts();
  };

  while (_read_buffer[_read_buffer_bytes_pending - 1] != '\n') {
    enable_interrupts();
  }
  _read_buffer[_read_buffer_bytes_pending - 1] = 0;

  strcpy(s, reinterpret_cast<char*>(_read_buffer));
  _read_buffer_bytes_pending = 0;
}

void MiniUART::putchar_async(const char c) {
  if (_write_buffer_bytes_pending >= WRITE_BUFFER_SIZE) {
    flush_write_buffer();
  }

  // Write the byte to the _write_buffer.
  _write_buffer[_write_buffer_bytes_pending++] = c;

  // Let mini UART interrupt the CPU when it becomes available,
  // and then we do the actual writing.
  //if (byte == '\n') {
  enable_interrupts();
  //}
}

void MiniUART::puts_async(const char* s, bool newline) {
  for (size_t i = 0; i < strlen(s); i++) {
    putchar_async(s[i]);
  }
  if (newline) {
    putchar_async('\n');
  }
}



bool MiniUART::is_buffer_enabled() const {
  return _is_buffer_enabled;
}

void MiniUART::set_buffer_enabled(bool enabled) {
  _is_buffer_enabled = enabled;
}

}  // namespace valkyrie::kernel

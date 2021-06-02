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
#include <driver/MiniUART.h>

#include <dev/Console.h>
#include <driver/IO.h>
#include <kernel/ExceptionManager.h>
#include <libs/CString.h>

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

#define BACKSPACE 0x7f

namespace valkyrie::kernel {

MiniUART& MiniUART::get_instance() {
  static MiniUART instance;
  return instance;
}

MiniUART::MiniUART()
    : _is_debugging(),
      _is_read_buffer_enabled(),
      _is_write_buffer_enabled(),
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
}


char MiniUART::read_char() {
  return getchar();
}

void MiniUART::write_char(const char c) {
  putchar(c);
}


char MiniUART::getchar() {
  return getchar_sync();
}

void MiniUART::gets(char* s) {
  (_is_read_buffer_enabled) ? gets_async(s) : gets_sync(s);
}

void MiniUART::putchar(const char c) {
  (_is_write_buffer_enabled) ? putchar_async(c) : putchar_sync(c);
}

void MiniUART::puts(const char* s, bool newline) {
  (_is_write_buffer_enabled) ? puts_async(s, newline) : puts_sync(s, newline);
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
        puts_sync("\b \b", /*newline=*/false);
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
  io::put<uint32_t>(ENABLE_IRQS_1, MINI_UART_IRQ);
}

void MiniUART::disable_interrupts() const {
  // Disable mini UART interrupt routing
  io::put<uint32_t>(DISABLE_IRQS_1, MINI_UART_IRQ);
}

bool MiniUART::has_pending_irq() const {
  return io::get<uint32_t>(IRQ_BASIC_PENDING) & IRQ_PENDING_1_HAS_PENDING_IRQ &&
         io::get<uint32_t>(IRQ_PENDING_1) & MINI_UART_IRQ;
}

void MiniUART::handle_irq() {
  bool has_pending_tx_irq = (io::get<uint32_t>(AUX_MU_IIR_REG) >> 1) & 0b01;
  bool has_pending_rx_irq = (io::get<uint32_t>(AUX_MU_IIR_REG) >> 1) & 0b10;

  if (has_pending_tx_irq) {
    handle_tx_irq();
  }
  if (has_pending_rx_irq) {
    handle_rx_irq();
  }

  disable_interrupts();
}


void MiniUART::handle_tx_irq() {
  flush_write_buffer();
}

void MiniUART::handle_rx_irq() {
  auto byte = io::get<uint8_t>(AUX_MU_IO_REG);

  if (byte == BACKSPACE) {
    if (_read_buffer_bytes_pending > 0) {
      _read_buffer_bytes_pending--;
      puts_async("\b \b", /*newline=*/false);
    }
  } else {
    byte = (byte == '\r') ? '\n' : byte;
    _read_buffer[_read_buffer_bytes_pending++] = byte;
    putchar_async(byte);
  }

  if (_is_debugging) {
    printf("[");
    for (int i = 0; i < _read_buffer_bytes_pending; i++) {
      printf("0x%x,", _read_buffer[i]);
    }
    printf("] (%d)\n", _read_buffer_bytes_pending);
  }
}

void MiniUART::flush_write_buffer() {
  for (int i = 0; i < _write_buffer_bytes_pending; i++) {
    putchar_sync(_write_buffer[i]);
  }
  _write_buffer_bytes_pending = 0;
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
  enable_interrupts();
}

void MiniUART::puts_async(const char* s, bool newline) {
  for (size_t i = 0; i < strlen(s); i++) {
    putchar_async(s[i]);
  }
  if (newline) {
    putchar_async('\n');
  }
}


bool MiniUART::is_debugging() const {
  return _is_debugging;
}

void MiniUART::set_debugging(bool debugging) {
  _is_debugging = debugging;
}

void MiniUART::set_read_buffer_enabled(bool enabled) {
  _is_read_buffer_enabled = enabled;
}

void MiniUART::set_write_buffer_enabled(bool enabled) {
  _is_write_buffer_enabled = enabled;
}

}  // namespace valkyrie::kernel

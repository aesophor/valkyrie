// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_POWER_H_
#define VALKYRIE_POWER_H_

#include <dev/IO.h>

#define PM_PASSWORD 0x5a000000
#define PM_RSTC     0x3F10001c
#define PM_WDOG     0x3F100024

namespace valkyrie::kernel {

// Causes the machine to reboot after watchdog timer expires.
inline void reset(int tick) {
  io::put(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
  io::put(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MINI_UART_H_

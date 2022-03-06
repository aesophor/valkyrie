// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_EXCEPTION_H_
#define VALKYRIE_EXCEPTION_H_

#include <Singleton.h>

#include <driver/IO.h>
#include <kernel/TaskletScheduler.h>
#include <proc/TrapFrame.h>

#define IRQ_BASIC_PENDING (MMIO_BASE + 0x0000B200)
#define IRQ_PENDING_1 (MMIO_BASE + 0x0000B204)
#define IRQ_PENDING_2 (MMIO_BASE + 0x0000B208)
#define FIQ_CONTROL (MMIO_BASE + 0x0000B20C)
#define ENABLE_IRQS_1 (MMIO_BASE + 0x0000B210)
#define ENABLE_IRQS_2 (MMIO_BASE + 0x0000B214)
#define ENABLE_BASIC_IRQS (MMIO_BASE + 0x0000B218)
#define DISABLE_IRQS_1 (MMIO_BASE + 0x0000B21C)
#define DISABLE_IRQS_2 (MMIO_BASE + 0x0000B220)
#define DISABLE_BASIC_IRQS (MMIO_BASE + 0x0000B224)

#define IRQ_PENDING_1_HAS_PENDING_IRQ (1 << 8)

#define SYSTEM_TIMER_IRQ_0 (1 << 0)
#define SYSTEM_TIMER_IRQ_1 (1 << 1)
#define SYSTEM_TIMER_IRQ_2 (1 << 2)
#define SYSTEM_TIMER_IRQ_3 (1 << 3)
#define MINI_UART_IRQ (1 << 29)

namespace valkyrie::kernel::exception {

extern bool _is_activated;

[[gnu::always_inline]] inline void enable_irqs() {
  asm volatile("msr DAIFCLR, #0b1111");
}

[[gnu::always_inline]] inline void disable_irqs() {
  asm volatile("msr DAIFSET, #0b1111");
}

[[gnu::always_inline]] inline bool is_activated() {
  return _is_activated;
}

[[gnu::always_inline]] inline void activate() {
  _is_activated = true;
}

}  // namespace valkyrie::kernel::exception

#endif  // VALKYRIE_EXCEPTION_H_

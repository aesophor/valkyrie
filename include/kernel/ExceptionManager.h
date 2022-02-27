// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_EXCEPTION_MANAGER_H_
#define VALKYRIE_EXCEPTION_MANAGER_H_

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

namespace valkyrie::kernel {

class ExceptionManager : public Singleton<ExceptionManager> {
 public:
  [[gnu::always_inline]] inline static void enableIRQs() {
    if (_is_activated) {
      asm volatile("msr DAIFCLR, #0b1111");
    }
  }

  [[gnu::always_inline]] inline static void disableIRQs() {
    asm volatile("msr DAIFSET, #0b1111");
  }

  static void handle_exception(TrapFrame *trap_frame);
  static void handle_irq(TrapFrame *trap_frame);

  [[gnu::always_inline]] inline uint8_t get_exception_level() const {
    // Note: CurrentEL is only accessible from EL1 or higher.
    uint8_t level;
    asm volatile("mrs %0, CurrentEL" : "=r"(level));
    return level >> 2;
  }

  [[gnu::always_inline]] inline static void activate() {
    _is_activated = true;
  }

  [[gnu::always_inline]] inline static bool is_activated() {
    return _is_activated;
  }

 protected:
  ExceptionManager() : _tasklet_scheduler() {}

 private:
  struct Exception final {
    uint8_t ec;    // exception class
    uint32_t iss;  // instruction specific syndrome
    size_t ret_addr;
  };

  static void dump_registers(TrapFrame *trap_frame);

  Exception get_current_exception();

  static bool _is_activated;
  TaskletScheduler _tasklet_scheduler;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_EXCEPTION_MANAGER_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_EXCEPTION_MANAGER_H_
#define VALKYRIE_EXCEPTION_MANAGER_H_

#include <dev/IO.h>
#include <kernel/TaskletScheduler.h>

#define IRQ_BASIC_PENDING  (MMIO_BASE + 0x0000B200)
#define IRQ_PENDING_1      (MMIO_BASE + 0x0000B204)
#define IRQ_PENDING_2      (MMIO_BASE + 0x0000B208)
#define FIQ_CONTROL        (MMIO_BASE + 0x0000B20C)
#define ENABLE_IRQS_1      (MMIO_BASE + 0x0000B210)
#define ENABLE_IRQS_2      (MMIO_BASE + 0x0000B214)
#define ENABLE_BASIC_IRQS  (MMIO_BASE + 0x0000B218)
#define DISABLE_IRQS_1     (MMIO_BASE + 0x0000B21C)
#define DISABLE_IRQS_2     (MMIO_BASE + 0x0000B220)
#define DISABLE_BASIC_IRQS (MMIO_BASE + 0x0000B224)

#define IRQ_PENDING_1_HAS_PENDING_IRQ (1 << 8)

#define SYSTEM_TIMER_IRQ_0 (1 << 0)
#define SYSTEM_TIMER_IRQ_1 (1 << 1)
#define SYSTEM_TIMER_IRQ_2 (1 << 2)
#define SYSTEM_TIMER_IRQ_3 (1 << 3)
#define MINI_UART_IRQ      (1 << 29)

namespace valkyrie::kernel {

class ExceptionManager final {
 public:
  static ExceptionManager& get_instance();
  ~ExceptionManager() = default;

  void enable();
  void disable();

  static void handle_exception(const size_t x0,
                               const size_t x1,
                               const size_t x2,
                               const size_t x3,
                               const size_t x4,
                               const size_t x5);
  static void handle_irq();

  uint8_t get_exception_level() const;
  void switch_to_exception_level(const uint8_t level,
                                 void* ret_addr = nullptr,
                                 void* new_sp = nullptr);

 private:
  ExceptionManager();

  struct Exception final {
    uint8_t ec;    // exception class
    uint32_t iss;  // instruction specific syndrome
    size_t ret_addr;
  };

  Exception get_current_exception();

  TaskletScheduler _tasklet_scheduler;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_EXCEPTION_MANAGER_H_

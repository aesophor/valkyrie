// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_EXCEPTION_MANAGER_H_
#define VALKYRIE_EXCEPTION_MANAGER_H_

#include <dev/IO.h>
#include <kernel/Timer.h>

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

#define SYSTEM_TIMER_IRQ_0 (1 << 0)
#define SYSTEM_TIMER_IRQ_1 (1 << 1)
#define SYSTEM_TIMER_IRQ_2 (1 << 2)
#define SYSTEM_TIMER_IRQ_3 (1 << 3)

namespace valkyrie::kernel {

class ExceptionManager final {
 public:
  static ExceptionManager* get_instance();
  ~ExceptionManager() = default;

  void enable();
  void disable();

  void handle_exception(const size_t number,
                        const size_t arg1,
                        const size_t arg2,
                        const size_t arg3,
                        const size_t arg4,
                        const size_t arg5,
                        const size_t arg6);
  void handle_irq();

  uint8_t get_exception_level() const;
  void switch_to_exception_level(const uint8_t level,
                                 const size_t new_sp = 0);

  ARMCoreTimer& get_arm_core_timer();

 private:
  ExceptionManager();

  struct Exception final {
    uint8_t ec;    // exception class
    uint32_t iss;  // instruction specific syndrome
    size_t ret_addr;
  };

  Exception get_current_exception();
 
  ARMCoreTimer _arm_core_timer;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_EXCEPTION_MANAGER_H_

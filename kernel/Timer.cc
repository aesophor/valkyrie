// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// * CNTPCT_EL0:    The timer’s current count.
//
// * CNTP_CVAL_EL0: A compared timer count.
//                  If CNTPCT_EL0 >= CNTP_CVAL_EL0, interrupt the CPU core.
//
// * CNTP_TVAL_EL0: (CNTP_CVAL_EL0 - CNTPCT_EL0).
//                  You can use it to set an expired timer after
//                  the current timer count.
//
// To set when the next timer IRQ will be triggered, the developer can write
// delta timer value to CNTP_TVAL_EL0, in that case CNTP_CVAL_EL0
// will be automatically populated with new value = CNTP_TVAL_EL0 + CNTPCT_EL0.
//
// Reference:
// [1] https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab4.html
// [2] https://lowenware.com/blog/osdev/aarch64-gic-and-timer-interrupt/
// [3] https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf

#include <kernel/Timer.h>

#define CORE0_TIMER_IRQ_CTRL       0x40000040
#define DEFAULT_TIMER_IRQ_INTERVAL 1  /* in seconds */

namespace valkyrie::kernel {

ARMCoreTimer::ARMCoreTimer()
    : _is_enabled(),
      _interval(DEFAULT_TIMER_IRQ_INTERVAL),
      _jiffies() {}


bool ARMCoreTimer::is_enabled() const {
  return _is_enabled;
}

void ARMCoreTimer::enable() {
  if (_is_enabled) {
    return;
  }

  // Enable ARM Core Timer.
  asm volatile("msr CNTP_CTL_EL0, %0" :: "r"(1));
  // Unmask timer interrupt.
  asm volatile("str %0, [%1]" :: "r"(0b0010), "r"(CORE0_TIMER_IRQ_CTRL));

  arrange_next_timer_irq_after(_interval);
  _is_enabled = true;
}

void ARMCoreTimer::disable() {
  if (!_is_enabled) {
    return;
  }

  // Disable ARM Core Timer.
  asm volatile("msr CNTP_CTL_EL0, %0" :: "r"(0));
  // Mask timer interrupt.
  asm volatile("str %0, [%1]" :: "r"(0b0000), "r"(CORE0_TIMER_IRQ_CTRL));

  _is_enabled = false;
}


void ARMCoreTimer::tick() {
  arrange_next_timer_irq_after(_interval);
  ++_jiffies;
}

void ARMCoreTimer::arrange_next_timer_irq_after(const uint32_t second) {
  uint64_t cntfrq_el0;
  asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(cntfrq_el0));
  //asm volatile("msr CNTP_TVAL_EL0, %0" :: "r"(cntfrq_el0 * second));
  asm volatile("msr CNTP_TVAL_EL0, %0" :: "r"(0x0ffff));
}


uint32_t ARMCoreTimer::get_jiffies() const {
  return _jiffies;
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TIMER_H_
#define VALKYRIE_TIMER_H_

#include <Types.h>

namespace valkyrie::kernel {

class ARMCoreTimer final {
 public:
  ARMCoreTimer();

  bool is_enabled() const;
  void enable();
  void disable();

  void tick();
  void arrange_next_timer_irq_after(const uint32_t second);

  uint32_t get_jiffies() const;

 private:
  bool _is_enabled;
  uint32_t _interval;
  uint32_t _jiffies;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TIMER_H_

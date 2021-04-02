// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TIMER_H_
#define VALKYRIE_TIMER_H_

#include <Types.h>
#include <dev/GPIO.h>

namespace valkyrie::kernel {

class ARMCoreTimer {
 public:
  ARMCoreTimer() : _jiffies(0) {}
  ~ARMCoreTimer() = default;

  void enable();
  void handle();

  uint32_t get_jiffies() const { return _jiffies; }
  void tick() { ++_jiffies; }

 private:
  uint32_t _jiffies;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TIMER_H_

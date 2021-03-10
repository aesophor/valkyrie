// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TIMER_H_
#define VALKYRIE_TIMER_H_

#include <GPIO.h>
#include <Types.h>

namespace valkyrie::kernel {

class ARMCoreTimer {
 public:
  ARMCoreTimer() : _jiffie() {}
  ~ARMCoreTimer() = default;

  void enable();
  void handle();

  size_t get_jiffie() const { return _jiffie; }
  void tick() { ++_jiffie; }

 private:
  [[gnu::aligned(16)]] size_t _jiffie;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TIMER_H_

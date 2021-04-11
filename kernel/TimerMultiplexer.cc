// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/TimerMultiplexer.h>

#include <kernel/Timer.h>

namespace valkyrie::kernel {

TimerMultiplexer& TimerMultiplexer::get_instance() {
  static TimerMultiplexer instance;
  return instance;
}

TimerMultiplexer::TimerMultiplexer()
    : _arm_core_timer(),
      _events() {}


void TimerMultiplexer::tick() {
  _arm_core_timer.tick();
  printk("ARM core timer interrupt: jiffies = %d\n", _arm_core_timer.get_jiffies());

  for (int i = 0; i < _events.size(); i++) {
    auto& ev = _events[i];
    --ev.timeout;

    if (ev.timeout == 0) {
      printk("event triggered: %s\n", ev.message.c_str());
    }
  }
}

void TimerMultiplexer::add_timer(const String& message,
                                 const uint32_t timeout) {
  printk("event registered. it will be triggered after %d secs\n", timeout);
  _events.push_back({message, timeout});
  _arm_core_timer.enable();
}

ARMCoreTimer& TimerMultiplexer::get_arm_core_timer() {
  return _arm_core_timer;
}

}  // namespace valkyrie::kernel

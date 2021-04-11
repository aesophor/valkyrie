// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TIMER_MULTIPLEXER_H_
#define VALKYRIE_TIMER_MULTIPLEXER_H_

//#include <Functional.h>
#include <String.h>
#include <Vector.h>
#include <kernel/Timer.h>

namespace valkyrie::kernel {

// Timers can be used to do periodic jobs such as scheduling, journaling
// and one-shot executed tasks such as sleeping and timeout.
// However, the number of the hardware timer is limited.
// Therefore, the kernel needs a software mechanism to multiplex the timer.
class TimerMultiplexer {
 public:
  static TimerMultiplexer& get_instance();
  ~TimerMultiplexer() = default;

  struct Event {
    //Function<void(Arguments)> callback;
    String message;
    uint32_t timeout;
  };

  void tick();

  void add_timer(const String& message,
                 const uint32_t timeout);

  ARMCoreTimer& get_arm_core_timer();

 private:
  explicit TimerMultiplexer();

  ARMCoreTimer _arm_core_timer;
  Vector<Event> _events;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TIMER_MULTIPLEXER_H_

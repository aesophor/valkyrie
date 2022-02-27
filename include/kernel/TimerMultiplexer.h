// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TIMER_MULTIPLEXER_H_
#define VALKYRIE_TIMER_MULTIPLEXER_H_

#include <Deque.h>
#include <Functional.h>
#include <Singleton.h>

#include <kernel/Timer.h>

namespace valkyrie::kernel {

// Timers can be used to do periodic jobs such as scheduling, journaling
// and one-shot executed tasks such as sleeping and timeout.
// However, the number of the hardware timer is limited.
// Therefore, the kernel needs a software mechanism to multiplex the timer.
class TimerMultiplexer : public Singleton<TimerMultiplexer> {
 public:
  struct Event {
    using Callback = Function<void()>;

    Callback callback;
    uint32_t timeout;
  };

  void tick();

  void add_timer(Event::Callback callback, const uint32_t timeout);

  ARMCoreTimer &get_arm_core_timer();

 protected:
  TimerMultiplexer();

 private:
  ARMCoreTimer _arm_core_timer;
  Deque<Event> _events;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TIMER_MULTIPLEXER_H_

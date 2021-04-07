// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SCHEDULER_H_
#define VALKYRIE_SCHEDULER_H_

#include <proc/Task.h>

namespace valkyrie::kernel {

class Scheduler {
 public:
  Scheduler();
  ~Scheduler() = default;

  void run();

 private:
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SCHEDULER_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASKLET_SCHEDULER_H_
#define VALKYRIE_TASKLET_SCHEDULER_H_

#include <Memory.h>
#include <RingBuffer.h>
#include <kernel/Tasklet.h>

namespace valkyrie::kernel {

class TaskletScheduler {
 public:
  TaskletScheduler();
  ~TaskletScheduler() = default;

  void schedule(UniquePtr<Tasklet> tasklet);
  void do_all();

 private:
  RingBuffer<UniquePtr<Tasklet>> _tasklet_queue;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASKLET_SCHEDULER_H_

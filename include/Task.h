// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <Types.h>

namespace valkyrie::kernel {

class Task {
 public:
  Task();
  ~Task() = default;

  uint32_t get_pid() const;

 private:
  static uint32_t _next_pid;
  uint32_t _pid;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_

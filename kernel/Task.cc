// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Task.h>

namespace valkyrie::kernel {

// PID starts at 1
uint32_t Task::_next_pid = 1;

Task::Task() : _pid(Task::_next_pid++) {}


uint32_t Task::get_pid() const {
  return _pid;
}

}  // namespace valkyrie::kernel

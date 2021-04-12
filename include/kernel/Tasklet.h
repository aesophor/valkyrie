// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASKLET_H_
#define VALKYRIE_TASKLET_H_

#include <Functional.h>

namespace valkyrie::kernel {

class Tasklet {
 public:
  using Handler = Function<void ()>;

  Tasklet() = default;
  ~Tasklet() = default;

  template <typename T>
  Tasklet(T&& t) : _handler(forward<T>(t)) {}

  void handle() {
    _handler();
  }

 private:
  Handler _handler;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASKLET_SCHEDULER_H_

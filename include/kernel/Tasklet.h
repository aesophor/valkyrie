// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASKLET_H_
#define VALKYRIE_TASKLET_H_

#include <Concepts.h>
#include <Functional.h>

namespace valkyrie::kernel {

class Tasklet {
 public:
  using Handler = Function<void ()>;

  // Universal reference constructor
  //
  // forwards any T as either lvalue or rvalue reference
  // where T::operator() is defined.
  template <Callable T>
  Tasklet(T&& t)
      : _handler(forward<T>(t)) {}

  // Destructor
  ~Tasklet() = default;

  // Copy constructor
  Tasklet(const Tasklet& r)
      : _handler(r._handler) {}

  // Copy assignment operator
  Tasklet& operator =(const Tasklet& r) {
    _handler = r._handler;
    return *this;
  }

  // Move constructor
  Tasklet(Tasklet&& r) noexcept
      : _handler(move(r._handler)) {}

  // Move assignment operator
  Tasklet& operator =(Tasklet&& r) noexcept {
    _handler = move(r._handler);
    return *this;
  }


  [[gnu::always_inline]] void handle() { _handler(); }

 private:
  Handler _handler;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASKLET_SCHEDULER_H_

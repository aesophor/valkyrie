// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASKLET_H_
#define VALKYRIE_TASKLET_H_

#include <Functional.h>

namespace valkyrie::kernel {

class Tasklet {
 public:
  using Handler = Function<void ()>;

  // Universal reference constructor
  //
  // forwards any T as either lvalue or rvalue reference
  // where T::operator() is defined.
  template <typename T> requires is_callable<T>
  Tasklet(T&& t) : _handler(forward<T>(t)) {}

  // Destructor
  ~Tasklet() = default;

  // Copy constructor
  Tasklet(const Tasklet& r) : _handler(r._handler) {}

  // Copy assignment operator
  Tasklet& operator =(const Tasklet& r) {
    _handler = r._handler;
    return *this;
  }

  // Move constructor
  Tasklet(Tasklet&& r) : _handler(move(r._handler)) {}

  // Move assignment operator
  Tasklet& operator =(Tasklet&& r) {
    _handler = move(r._handler);
    return *this;
  }


  void handle() {
    _handler();
  }

 private:
  Handler _handler;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASKLET_SCHEDULER_H_

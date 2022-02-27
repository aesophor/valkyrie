// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
// WARNING: DO NOT USE THIS WITHOUT VIRTUAL MEMORY
#ifndef VALKYRIE_ATOMIC_H_
#define VALKYRIE_ATOMIC_H_

#include <Utility.h>

namespace valkyrie::kernel {

template <typename>
class Atomic;


// Partial specialization of class `Atomic`
template <>
class Atomic<bool> {
 public:
  // Default constructor
  Atomic() : _v() {}

  // Constructor
  explicit
  Atomic(bool v) : _v(v) {}

  // Destructor
  ~Atomic() = default;

  // Copy constructor
  Atomic(const Atomic& r) = delete;

  // Copy assignment operator
  Atomic& operator= (const Atomic& r) = delete;

  // Move constructor
  Atomic(Atomic&& r) noexcept { *this = move(r); }

  // Move assignment operator
  Atomic& operator= (Atomic&& r) noexcept {
    static_cast<void>(__sync_lock_test_and_set(&_v, r._v));
    return *this;
  }

  Atomic& operator= (bool v) {
    static_cast<void>(__sync_lock_test_and_set(&_v, v));
    return *this;
  }

 private:
  volatile bool _v;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ATOMIC_H_

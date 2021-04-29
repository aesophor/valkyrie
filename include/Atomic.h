// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ATOMIC_H_
#define VALKYRIE_ATOMIC_H_

namespace valkyrie::kernel {

template <typename>
class Atomic;

// Partial specialization of class `Atomic`
template <>
class Atomic<int> {
 public:
  Atomic();
  ~Atomic() = default;

 private:
  void inc() { __sync_add_and_fetch(&_v, 1); }
  void dec() { __sync_sub_and_fetch(&_v, 1); }

  int _v;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ATOMIC_H_

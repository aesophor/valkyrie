// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UNIQUE_LOCK_H_
#define VALKYRIE_UNIQUE_LOCK_H_

namespace valkyrie::kernel {

template <typename T>
class UniqueLock {
 public:
  UniqueLock(T& t) : _t(t) { _t.lock(); }
  ~UniqueLock() { _t.unlock(); }

 private:
  T& _t;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UNIQUE_LOCK_H_

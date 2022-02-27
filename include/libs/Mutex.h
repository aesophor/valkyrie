// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// The mutex class is a synchronization primitive that can be used to
// protect shared data from being simultaneously accessed by multiple threads.
#ifndef VALKYRIE_MUTEX_H_
#define VALKYRIE_MUTEX_H_

#include <kernel/ExceptionManager.h>

namespace valkyrie::kernel {

class RecursiveMutex {
 public:
  RecursiveMutex() : _is_locked(), _depth() {}
  ~RecursiveMutex() = default;

  // Locks the mutex, blocks if the mutex is not available
  void lock() {
    if (ExceptionManager::is_activated()) [[likely]] {
      ExceptionManager::disableIRQs();
      _is_locked = true;
      _depth++;
    }
  }

  // Unlocks the mutex.
  void unlock() {
    if (ExceptionManager::is_activated()) [[likely]] {
      if (!--_depth) {
        _is_locked = false;
        ExceptionManager::enableIRQs();
      }
    }

    // XXX: This must not happen...
    if (_depth < 0) {
      while (1)
        ;
    }
  }

 private:
  bool _is_locked;
  int _depth;
};

template <typename T>
class LockGuard {
 public:
  LockGuard(T &t) : _t(t) {
    _t.lock();
  }
  ~LockGuard() {
    _t.unlock();
  }

 private:
  T &_t;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MUTEX_H_

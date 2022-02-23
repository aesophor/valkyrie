// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// The mutex class is a synchronization primitive that can be used to
// protect shared data from being simultaneously accessed by multiple threads.
#ifndef VALKYRIE_MUTEX_H_
#define VALKYRIE_MUTEX_H_

#include <kernel/ExceptionManager.h>

namespace valkyrie::kernel {

class Mutex {
 public:
  Mutex() : _is_locked() {}
  ~Mutex() = default;

  // Locks the mutex, blocks if the mutex is not available
  [[gnu::always_inline]]
  inline void lock() {
    ExceptionManager::disableIRQs();
    _is_locked = true;
  }

  // Tries to lock the mutex,
  // returns if the mutex is not available.
  [[gnu::always_inline]]
  inline void try_lock() {

  }

  // Unlocks the mutex.
  [[gnu::always_inline]]
  inline void unlock() {
    _is_locked = false;
    ExceptionManager::enableIRQs();
  }

 private:
  volatile bool _is_locked;
};


class RecursiveMutex {
 public:
  RecursiveMutex() : _is_locked(), _depth() {}
  ~RecursiveMutex() = default;

  // Locks the mutex, blocks if the mutex is not available
  [[gnu::always_inline]]
  inline void lock() {
    ExceptionManager::disableIRQs();
    _is_locked = true;
    _depth++;
  }

  // Unlocks the mutex.
  [[gnu::always_inline]]
  inline void unlock() {
    _depth--;
    if (_depth == 0) {
      _is_locked = false;
      ExceptionManager::enableIRQs();
    }
    // XXX: This must not happen...
    if (_depth < 0) {
      while (1);
    }
  }

 private:
  bool _is_locked;
  int _depth;
};


template <typename T>
class LockGuard {
 public:
  LockGuard(T& t) : _t(t) { _t.lock(); }
  ~LockGuard() { _t.unlock(); }

 private:
  T& _t;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MUTEX_H_

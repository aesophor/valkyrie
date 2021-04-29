// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// The mutex class is a synchronization primitive that can be used to
// protect shared data from being simultaneously accessed by multiple threads.
#ifndef VALKYRIE_MUTEX_H_
#define VALKYRIE_MUTEX_H_

namespace valkyrie::kernel {

class Mutex {
 public:
  Mutex();
  ~Mutex();

  void lock();
  void try_lock();
  void unlock();

 private:
  bool _is_locked;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MUTEX_H_

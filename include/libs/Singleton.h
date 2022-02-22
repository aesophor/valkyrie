// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// Singleton.h - Abstract base class of all singleton class.
//
// The `Singleton` abstract base class saves us from
// manually declaring the the() static method
// for each singleton class.
//
// In order to define a singleton class, write:
// class X : public Singleton<X> { ... };
//
// where X's constructor musted be marked "protected".

#ifndef VALKYRIE_SINGLETON_H_
#define VALKYRIE_SINGLETON_H_

#include <TypeTraits.h>

namespace valkyrie::kernel {

template <typename T>
class Singleton {
  MAKE_NONCOPYABLE(Singleton);
  MAKE_NONMOVABLE(Singleton);

 public:
  static T& the() {
    // A wrapper struct inheriting from T
    // which grants access to T's protected constructor.
    static struct 🐱 : public T {
      [[gnu::always_inline]] 🐱() : T() {}
    } instance;

    return instance;
  }

 protected:
  Singleton() = default;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SINGLETON_H_

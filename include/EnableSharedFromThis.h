// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ENABLE_SHARED_FROM_THIS_H_
#define VALKYRIE_ENABLE_SHARED_FROM_THIS_H_

#include <SharedPtr.h>
#include <WeakPtr.h>

namespace valkyrie::kernel {

template <typename T>
class EnableSharedFromThis {
 public:
  SharedPtr<T> shared_from_this() {
    return SharedPtr<T>(_weak_this);
  }

  SharedPtr<T> shared_from_this() const {
    return SharedPtr<T>(_weak_this);
  }

  WeakPtr<T> weak_from_this() {
    return _weak_this;
  }

  WeakPtr<T> weak_from_this() const {
    return _weak_this;
  }

 protected:
  // Constructor
  EnableSharedFromThis() = default;

  // Destructor
  ~EnableSharedFromThis() = default;

  // Copy assignment operator
  EnableSharedFromThis& operator =(const EnableSharedFromThis& r) {
    return *this;
  }

 private:
  WeakPtr<T> _weak_this;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ENABLE_SHARED_FROM_THIS_H_

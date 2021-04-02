// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UNIQUE_PTR_H_
#define VALKYRIE_UNIQUE_PTR_H_

#include <Utility.h>

namespace valkyrie::kernel {

template <typename T>
class UniquePtr {
 public:
  UniquePtr() : _p() {}
  explicit UniquePtr(T* p) : _p(p) {}
  ~UniquePtr() { reset(); }

  UniquePtr(UniquePtr&& other) noexcept : _p(other.release()) {}

  UniquePtr& operator=(UniquePtr&& other) noexcept {
    reset(other.release());
    return *this;
  }

  UniquePtr(const UniquePtr&) = delete;
  UniquePtr& operator=(const UniquePtr&) = delete;


  T* operator ->() const { return get(); }

  T* get() const { return _p; }

  void reset(T* p = nullptr) {
    if (p == _p) {
      return;
    }
    delete _p;
    _p = nullptr;
  }

  T* release() {
    T* p = _p;
    _p = nullptr;
    return p;
  }

 private:
  T* _p;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UNIQUE_PTR_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UNIQUE_PTR_H_
#define VALKYRIE_UNIQUE_PTR_H_

#include <Types.h>
#include <Utility.h>

namespace valkyrie::kernel {

template <typename T>
class UniquePtr {
 public:
  UniquePtr()
    : _p() {}

  explicit UniquePtr(T* p)
    : _p(p) {}

  ~UniquePtr() { reset(); }

  // Move constructor
  UniquePtr(UniquePtr&& other) noexcept
    : _p(other.release()) {}

  // Move assignment operator
  UniquePtr& operator =(UniquePtr&& other) noexcept {
    reset(other.release());
    return *this;
  }

  // Copy constructor
  UniquePtr(const UniquePtr&) = delete;

  // Copy assignment operator
  UniquePtr& operator =(const UniquePtr&) = delete;


  T* operator ->() const { return get(); }
  T& operator *() const { return *get(); }
  operator bool() const { return static_cast<bool>(_p); }

  T* get() const { return _p; }

  void reset(T* p = nullptr) {
    if (p == _p) {
      return;
    }
    delete _p;
    _p = p;
  }

  T* release() {
    T* p = _p;
    _p = nullptr;
    return p;
  }

 private:
  T* _p;
};


template <typename T>
struct _Unique_if { using _SingleObject = UniquePtr<T>; };

template <typename T>
struct _Unique_if<T[]> { using _UnknownBound = UniquePtr<T[]>; };

template <typename T, size_t N>
struct _Unique_if<T[N]> { using _KnownBound = void; };

template <typename T, typename... Args>
typename _Unique_if<T>::_SingleObject make_unique(Args&&... args) {
  return UniquePtr<T>(new T(forward<Args>(args)...));
}

template <typename T>
typename _Unique_if<T>::_UnknownBound make_unique(size_t n) {
  using U = RemoveExtent<T>;
  return UniquePtr<T>(new U[n]());
}

template <typename T, typename... Args>
typename _Unique_if<T>::_KnownBound make_unique(Args&&...) = delete;

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UNIQUE_PTR_H_

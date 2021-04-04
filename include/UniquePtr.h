// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UNIQUE_PTR_H_
#define VALKYRIE_UNIQUE_PTR_H_

#include <Types.h>
#include <Utility.h>

namespace valkyrie::kernel {

template <typename T>
class UniquePtr {
 public:
  // Default Constructor
  UniquePtr() : _p() {}

  // Constructor
  explicit
  UniquePtr(T* p) : _p(p) {}

  // Destructor
  ~UniquePtr() { reset(); }

  // Copy constructor
  UniquePtr(const UniquePtr&) = delete;

  // Copy assignment operator
  UniquePtr& operator =(const UniquePtr&) = delete;

  // Move constructor
  UniquePtr(UniquePtr&& other) noexcept : _p(other.release()) {}

  // Move assignment operator
  UniquePtr& operator =(UniquePtr&& other) noexcept {
    reset(other.release());
    return *this;
  }


  T* operator ->() const { return get(); }
  T& operator *() const { return *get(); }
  operator bool() const { return get(); }

  T* get() const { return _p; }

  void reset(T* p = nullptr) {
    if (_p == p) {
      return;
    }
    delete _p;
    _p = p;
  }

  void swap(UniquePtr& r) noexcept {
    using ::valkyrie::kernel::swap;
    swap(_p, r._p);
  }

  T* release() {
    T* p = _p;
    _p = nullptr;
    return p;
  }

 private:
  T* _p;
};


// https://stackoverflow.com/questions/47360599/c-is-there-a-way-for-a-template-class-specialization-to-contain-code-from-th
template <typename T>
class UniquePtr<T[]> {
 public:
  // Default Constructor
  UniquePtr() : _p() {}

  // Constructor
  explicit
  UniquePtr(T* p) : _p(p) {}

  // Destructor
  ~UniquePtr() { reset(); }

  // Copy constructor
  UniquePtr(const UniquePtr&) = delete;

  // Copy assignment operator
  UniquePtr& operator =(const UniquePtr&) = delete;

  // Move constructor
  UniquePtr(UniquePtr&& other) noexcept : _p(other.release()) {}

  // Move assignment operator
  UniquePtr& operator =(UniquePtr&& other) noexcept {
    reset(other.release());
    return *this;
  }

  T& operator [](size_t i) { return get()[i]; }
  T* operator ->() const { return get(); }
  T& operator *() const { return *get(); }
  operator bool() const { return get(); }

  T* get() const { return _p; }

  void reset(T* p = nullptr) {
    if (_p == p) {
      return;
    }
    delete[] _p;
    _p = p;
  }

  void swap(UniquePtr& r) noexcept {
    using ::valkyrie::kernel::swap;
    swap(_p, r._p);
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
struct _UniqueIf { using _SingleObject = UniquePtr<T>; };

template <typename T>
struct _UniqueIf<T[]> { using _UnknownBound = UniquePtr<T[]>; };

template <typename T, size_t N>
struct _UniqueIf<T[N]> { using _KnownBound = void; };

template <typename T, typename... Args>
typename _UniqueIf<T>::_SingleObject make_unique(Args&&... args) {
  return UniquePtr<T>(new T(forward<Args>(args)...));
}

template <typename T>
typename _UniqueIf<T>::_UnknownBound make_unique(size_t n) {
  using U = RemoveExtent<T>;
  return UniquePtr<T>(new U[n]());
}

template <typename T, typename... Args>
typename _UniqueIf<T>::_KnownBound make_unique(Args&&...) = delete;

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UNIQUE_PTR_H_

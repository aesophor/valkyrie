// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UNIQUE_PTR_H_
#define VALKYRIE_UNIQUE_PTR_H_

#include <Hash.h>
#include <TypeTraits.h>
#include <Utility.h>

namespace valkyrie::kernel {

template <typename T>
class UniquePtr {
 protected:
  MAKE_NONCOPYABLE(UniquePtr);

  // Friend declaration
  friend struct Hash<UniquePtr<T>>;

 public:
  // Default constructor
  UniquePtr() : _p() {}

  // Constructor
  explicit UniquePtr(T *p) : _p(p) {}

  // Constructor (from a nullptr_t)
  UniquePtr(nullptr_t) : _p() {}

  // Destructor
  ~UniquePtr() {
    reset();
  }

  // Move constructor
  UniquePtr(UniquePtr &&r) noexcept : _p(r.release()) {}

  // Move assignment operator
  UniquePtr &operator=(UniquePtr &&r) noexcept {
    reset(r.release());
    return *this;
  }

  // Conversion operator
  template <typename U>
  operator UniquePtr<U>() {
    if (dynamic_cast<U *>(_p)) {
      return UniquePtr<U>(release());
    }
    return UniquePtr<U>();
  }

  T *operator->() const {
    return get();
  }

  T &operator*() const {
    return *get();
  }

  operator bool() const {
    return get();
  }

  bool operator==(UniquePtr r) const {
    return _p == r._p;
  }

  bool operator!=(UniquePtr r) const {
    return _p != r._p;
  }

  T *get() const {
    return _p;
  }

  void reset(T *p = nullptr) {
    if (_p == p) {
      return;
    }
    delete _p;
    _p = p;
  }

  void swap(UniquePtr &r) noexcept {
    using ::valkyrie::kernel::swap;
    swap(*this, r);
  }

  T *release() {
    T *p = _p;
    _p = nullptr;
    return p;
  }

 protected:
  T *_p;
};

template <typename T>
class UniquePtr<T[]> : private UniquePtr<T> {
  using UniquePtr<T>::UniquePtr;
  using UniquePtr<T>::operator=;

 public:
  // Move constructor
  UniquePtr(UniquePtr &&other) noexcept {
    *this = move(other);
  }

  // Move assignment operator
  UniquePtr &operator=(UniquePtr &&other) noexcept {
    reset(other.release());
    return *this;
  }

  ~UniquePtr() {
    reset();
  }

  T &operator[](size_t i) {
    return get()[i];
  }

  const T &operator[](size_t i) const {
    return get()[i];
  }

  using UniquePtr<T>::operator->;
  using UniquePtr<T>::operator*;
  using UniquePtr<T>::operator bool;
  using UniquePtr<T>::operator==;
  using UniquePtr<T>::operator!=;

  using UniquePtr<T>::get;
  using UniquePtr<T>::release;
  using UniquePtr<T>::swap;

  void reset(T *p = nullptr) {
    if (_p == p) {
      return;
    }
    delete[] _p;
    _p = p;
  }

 private:
  using UniquePtr<T>::_p;
};

template <typename T>
struct _UniqueIf {
  using _SingleObject = UniquePtr<T>;
};

template <typename T>
struct _UniqueIf<T[]> {
  using _UnknownBound = UniquePtr<T[]>;
};

template <typename T, size_t N>
struct _UniqueIf<T[N]> {
  using _KnownBound = void;
};

template <typename T, typename... Args>
typename _UniqueIf<T>::_SingleObject make_unique(Args &&...args) {
  return UniquePtr<T>(new T(forward<Args>(args)...));
}

template <typename T>
typename _UniqueIf<T>::_UnknownBound make_unique(size_t n) {
  return UniquePtr<T>(new RemoveExtent<T>[n]());
}

template <typename T, typename... Args>
typename _UniqueIf<T>::_KnownBound make_unique(Args &&...) = delete;

// Explicit (full) specialization of struct `Hash` for UniquePtr<T>
template <typename T>
struct Hash<UniquePtr<T>> {
  size_t operator()(const UniquePtr<T> &up) const {
    constexpr size_t prime = 11;
    size_t ret = 29;

    ret += prime * hash(up._p);
    return ret;
  }
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UNIQUE_PTR_H_

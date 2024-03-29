// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SHARED_PTR_H_
#define VALKYRIE_SHARED_PTR_H_

#include <Hash.h>
#include <TypeTraits.h>
#include <UniquePtr.h>

namespace valkyrie::kernel {

// Forward declaration
template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

template <typename T>
class SharedPtr {
  // Friend declaration
  template <typename U>
  friend class SharedPtr;  // used by the aliasing constructor

  template <typename U>
  friend class WeakPtr;

  friend struct Hash<SharedPtr<T>>;

 public:
  // Default constructor
  SharedPtr() : _ctrl(), _alias() {}

  // Constructor (from a nullptr_t)
  SharedPtr(nullptr_t) : _ctrl(), _alias() {}

  // Constructor (from a raw pointer of type T)
  explicit SharedPtr(T *p) : _ctrl(new ControlBlock{p, 1, 0}), _alias() {
    maybe_enable_shared_from_this();
  }

  // Constructor (from an UniquePtr<T>)
  explicit SharedPtr(UniquePtr<T> &&r)
      : _ctrl(new ControlBlock{r.release(), 1, 0}), _alias() {}

  // Constructor (from a WeakPtr<T>)
  explicit SharedPtr(const WeakPtr<T> &r) : _ctrl(r._ctrl), _alias() {
    inc_use_count();
  }

  // Aliasing Constructor
  // It allows us to construct a new SharedPtr instance
  // that shares ownership with another SharedPtr `r`,
  // but with a different pointer value.
  template <typename U>
  SharedPtr(const SharedPtr<U> &r, T *alias_ptr)
      : _ctrl(reinterpret_cast<ControlBlock *>(r._ctrl)), _alias(alias_ptr) {
    inc_use_count();
  }

  // Destructor
  ~SharedPtr() {
    dec_use_count();
  }

  // Copy constructor
  SharedPtr(const SharedPtr &r) : _ctrl(r._ctrl), _alias(r._alias) {
    inc_use_count();
  }

  // Copy assignment operator
  SharedPtr &operator=(const SharedPtr &r) {
    if (_ctrl && _ctrl->use_count > 0) {
      reset();
    }

    _ctrl = r._ctrl;
    _alias = r._alias;
    inc_use_count();
    return *this;
  }

  // Move constructor
  SharedPtr(SharedPtr &&r) noexcept : _ctrl(r._ctrl), _alias(r._alias) {
    r._ctrl = nullptr;
    r._alias = nullptr;
  }

  // Move assignment operator
  SharedPtr &operator=(SharedPtr &&r) noexcept {
    if (_ctrl && _ctrl->use_count > 0) {
      reset();
    }

    _ctrl = r._ctrl;
    _alias = r._alias;
    r._ctrl = nullptr;
    r._alias = nullptr;
    return *this;
  }

  // Conversion operator (dynamic_cast)
  template <typename U>
  operator SharedPtr<U>() const {
    return dynamic_pointer_cast<U>(*this);
  }

  operator WeakPtr<T>() const {
    return WeakPtr<T>(*this);
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

  bool operator==(SharedPtr r) const {
    return _ctrl == r._ctrl;
  }

  bool operator!=(SharedPtr r) const {
    return _ctrl != r._ctrl;
  }

  T *get() const {
    if (_alias) {
      return _alias;
    }
    return (_ctrl) ? _ctrl->p : nullptr;
  }

  void reset(T *p = nullptr) {
    SharedPtr<T>(p).swap(*this);
  }

  void swap(SharedPtr &r) noexcept {
    using ::valkyrie::kernel::swap;
    swap(*this, r);
  }

  int use_count() const {
    return (_ctrl) ? _ctrl->use_count : 0;
  }

 protected:
  void inc_use_count() {
    if (_ctrl) {
      _ctrl->use_count++;
    }
  }

  void dec_use_count() {
    if (_ctrl && _ctrl->use_count > 0) {
      _ctrl->use_count--;
      maybe_delete_p();
      maybe_delete_ctrl_block();
    }
  }

  void maybe_delete_p() {
    if (_ctrl && _ctrl->use_count == 0) {
      delete _ctrl->p;
      _ctrl->p = nullptr;
    }
  }

  void maybe_delete_ctrl_block() {
    if (_ctrl && _ctrl->use_count + _ctrl->use_count_weak == 0) {
      // Set use_count_weak to -1 to prevent against double delete.
      // FIXME: thread safety
      _ctrl->use_count_weak = -1;

      delete _ctrl;
      _ctrl = nullptr;
    }
  }

  [[gnu::always_inline]] void maybe_enable_shared_from_this() const {
    // If class `EnableSharedFromThis<T>` is the base class of `T`,
    // then compile the following statement which initialize
    // `EnableSharedFromThis<T>::_weak_this`, so that
    // the user may call shared_from_this() later.
    if constexpr (IsBaseOf<EnableSharedFromThis<T>, T>::value) {
      _ctrl->p->_weak_this = *this;
    }
  }

  struct ControlBlock final {
    T *p;
    int use_count;
    int use_count_weak;
  } * _ctrl;

  // For SharedPtr's aliasing constructor.
  T *_alias;
};

template <typename T>
class SharedPtr<T[]> : private SharedPtr<T> {
 public:
  using SharedPtr<T>::SharedPtr;
  using SharedPtr<T>::operator=;

  // Move constructor
  SharedPtr(SharedPtr &&other) noexcept {
    *this = move(other);
  }

  // Move assignment operator
  SharedPtr &operator=(SharedPtr &&other) noexcept {
    if (_ctrl && _ctrl->use_count > 0) {
      reset();
    }

    _ctrl = other._ctrl;
    _alias = other._alias;
    other._ctrl = nullptr;
    other._alias = nullptr;
    return *this;
  }

  ~SharedPtr() {
    dec_use_count();
  }

  T &operator[](size_t i) {
    return get()[i];
  }

  const T &operator[](size_t i) const {
    return get()[i];
  }

  using SharedPtr<T>::operator->;
  using SharedPtr<T>::operator*;
  using SharedPtr<T>::operator bool;
  using SharedPtr<T>::operator==;
  using SharedPtr<T>::operator!=;

  using SharedPtr<T>::get;
  using SharedPtr<T>::reset;
  using SharedPtr<T>::swap;
  using SharedPtr<T>::use_count;

 private:
  using SharedPtr<T>::inc_use_count;
  using SharedPtr<T>::dec_use_count;
  using SharedPtr<T>::maybe_delete_ctrl_block;

  void maybe_delete_p() {
    if (_ctrl && _ctrl->use_count == 0) {
      delete[] _ctrl->p;
      _ctrl->p = nullptr;
    }
  }

  using SharedPtr<T>::_ctrl;
  using SharedPtr<T>::_alias;
};

template <typename T>
struct _SharedIf {
  using _SingleObject = SharedPtr<T>;
};

template <typename T>
struct _SharedIf<T[]> {
  using _UnknownBound = SharedPtr<T[]>;
};

template <typename T, size_t N>
struct _SharedIf<T[N]> {
  using _KnownBound = void;
};

template <typename T, typename... Args>
typename _SharedIf<T>::_SingleObject make_shared(Args &&...args) {
  return SharedPtr<T>(new T(forward<Args>(args)...));
}

template <typename T>
typename _SharedIf<T>::_UnknownBound make_shared(size_t n) {
  return SharedPtr<T>(new RemoveExtent<T>[n]());
}

template <typename T, typename... Args>
typename _SharedIf<T>::_KnownBound make_shared(Args &&...) = delete;

template <typename T, typename U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U> &r) noexcept {
  auto p = static_cast<T *>(r.get());
  return SharedPtr<T>(r, p);
}

template <typename T, typename U>
SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &r) noexcept {
  // NOTE: Downcast is impossible due to -fno-rtti
  if (auto p = dynamic_cast<T *>(r.get())) {
    return SharedPtr<T>(r, p);
  }
  return SharedPtr<T>();
}

template <typename T, typename U>
SharedPtr<T> const_pointer_cast(const SharedPtr<U> &r) noexcept {
  auto p = const_cast<T *>(r.get());
  return SharedPtr<T>(r, p);
}

template <typename T, typename U>
SharedPtr<T> reinterpret_pointer_cast(const SharedPtr<U> &r) noexcept {
  auto p = reinterpret_cast<T *>(r.get());
  return SharedPtr<T>(r, p);
}

template <typename T>
class EnableSharedFromThis {
  // Friend declaration
  friend class SharedPtr<T>;

 public:
  SharedPtr<T> shared_from_this() {
    return _weak_this.lock();
  }

  SharedPtr<T> shared_from_this() const {
    return _weak_this.lock();
  }

  WeakPtr<T> weak_from_this() {
    return _weak_this;
  }

  WeakPtr<T> weak_from_this() const {
    return _weak_this;
  }

 protected:
  // Constructor
  constexpr EnableSharedFromThis() = default;

  // Destructor
  ~EnableSharedFromThis() = default;

  // Copy assignment operator
  EnableSharedFromThis &operator=(const EnableSharedFromThis &r) {
    return *this;
  }

 private:
  WeakPtr<T> _weak_this;
};

// Explicit (full) specialization of struct `Hash` for SharedPtr<T>
template <typename T>
struct Hash<SharedPtr<T>> {
  size_t operator()(const SharedPtr<T> &sp) const {
    constexpr size_t prime = 23;
    size_t ret = 17;

    ret += prime * hash(sp._ctrl);
    ret += prime * hash(sp._alias);
    return ret;
  }
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SHARED_PTR_H_

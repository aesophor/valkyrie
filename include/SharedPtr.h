// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SHARED_PTR_H_
#define VALKYRIE_SHARED_PTR_H_

#include <Types.h>
#include <UniquePtr.h>

namespace valkyrie::kernel {

template <typename T>
class SharedPtr {
 public:
  // Default Constructor
  SharedPtr() : _ctrl() {}

  // Constructor (from a raw pointer)
  explicit
  SharedPtr(T* p) : _ctrl(new ControlBlock(p, 1)) {}

  // Constructor (from an UniquePtr<T>)
  explicit
  SharedPtr(UniquePtr<T>&& r) : _ctrl(new ControlBlock(r.release(), 1)) {}

  // Destructor
  ~SharedPtr() { dec_use_count(); }

  // Copy constructor
  SharedPtr(const SharedPtr& other) : _ctrl(other._ctrl) {
    inc_use_count();
  }

  // Copy assignment operator
  SharedPtr& operator =(const SharedPtr& other) {
    reset(other);
    return *this;
  }

  // Move constructor
  SharedPtr(SharedPtr&& other) noexcept {
    *this = move(other);
  }

  // Move assignment operator
  SharedPtr& operator =(SharedPtr&& other) noexcept {
    if (is_valid()) {
      dec_use_count();
    }
    _ctrl = other._ctrl;
    other._ctrl = nullptr;
    return *this;
  }


  T* operator ->() const { return get(); }
  T& operator *() const { return *get(); }
  operator bool() const { return get(); }

  T* get() const {
    return (_ctrl) ? _ctrl->p : nullptr;
  }

  void reset(T* p = nullptr) {
    SharedPtr<T>(p).swap(*this);
  }

  void swap(SharedPtr& r) noexcept {
    using ::valkyrie::kernel::swap;
    swap(_ctrl, r._ctrl);
  }

  int use_count() const {
    return (is_valid()) ? _ctrl->use_count : 0;
  }

 private:
  void inc_use_count() {
    if (is_valid()) {
      ++(_ctrl->use_count);
    }
  }

  void dec_use_count() {
    if (is_valid()) {
      --(_ctrl->use_count);

      if (use_count() <= 0) {
        delete _ctrl->p;
        delete _ctrl;
      }
    }
  }

  bool is_valid() const {
    return _ctrl;
  }


  struct ControlBlock {
    ControlBlock(T* p = nullptr, int use_count = 0)
        : p(p), use_count(use_count) {}

    T* p;
    int use_count;
  }* _ctrl;
};


// https://stackoverflow.com/questions/47360599/c-is-there-a-way-for-a-template-class-specialization-to-contain-code-from-th
template <typename T>
class SharedPtr<T[]> {
 public:
  // Default Constructor
  SharedPtr() : _ctrl() {}

  // Constructor (from a raw pointer)
  explicit
  SharedPtr(T* p) : _ctrl(new ControlBlock(p, 1)) {}

  // Constructor (from an UniquePtr<T>)
  explicit
  SharedPtr(UniquePtr<T>&& r) : _ctrl(new ControlBlock(r.release(), 1)) {}

  // Destructor
  ~SharedPtr() { dec_use_count(); }

  // Copy constructor
  SharedPtr(const SharedPtr& other) : _ctrl(other._ctrl) {
    inc_use_count();
  }

  // Copy assignment operator
  SharedPtr& operator =(const SharedPtr& other) {
    reset(other);
    return *this;
  }

  // Move constructor
  SharedPtr(SharedPtr&& other) noexcept {
    *this = move(other);
  }

  // Move assignment operator
  SharedPtr& operator =(SharedPtr&& other) noexcept {
    if (is_valid()) {
      dec_use_count();
    }
    _ctrl = other._ctrl;
    other._ctrl = nullptr;
    return *this;
  }


  T& operator [](size_t i) { return get()[i]; }
  T* operator ->() const { return get(); }
  T& operator *() const { return *get(); }
  operator bool() const { return get(); }

  T* get() const {
    return (_ctrl) ? _ctrl->p : nullptr;
  }

  void reset(T* p = nullptr) {
    SharedPtr<T>(p).swap(*this);
  }

  void swap(SharedPtr& r) noexcept {
    using ::valkyrie::kernel::swap;
    swap(_ctrl, r._ctrl);
  }

  int use_count() const {
    return (is_valid()) ? _ctrl->use_count : 0;
  }

 private:
  void inc_use_count() {
    if (is_valid()) {
      ++(_ctrl->use_count);
    }
  }

  void dec_use_count() {
    if (is_valid()) {
      --(_ctrl->use_count);

      if (use_count() <= 0) {
        delete[] _ctrl->p;
        delete _ctrl;
      }
    }
  }

  bool is_valid() const {
    return _ctrl;
  }


  struct ControlBlock {
    ControlBlock(T* p = nullptr, int use_count = 0)
        : p(p), use_count(use_count) {}

    T* p;
    int use_count;
  }* _ctrl;
};

template <typename T>
struct _SharedIf { using _SingleObject = SharedPtr<T>; };

template <typename T>
struct _SharedIf<T[]> { using _UnknownBound = SharedPtr<T[]>; };

template <typename T, size_t N>
struct _SharedIf<T[N]> { using _KnownBound = void; };


template <typename T, typename... Args>
typename _SharedIf<T>::_SingleObject make_shared(Args&&... args) {
  return SharedPtr<T>(new T(forward<Args>(args)...));
}

template <typename T>
typename _SharedIf<T>::_UnknownBound make_shared(size_t n) {
  using U = RemoveExtent<T>;
  return SharedPtr<T>(new U[n]());
}

template <typename T, typename... Args>
typename _SharedIf<T>::_KnownBound make_shared(Args&&...) = delete;

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SHARED_PTR_H_

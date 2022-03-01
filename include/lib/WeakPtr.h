// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_WEAK_PTR_H_
#define VALKYRIE_WEAK_PTR_H_

#include <Hash.h>
#include <SharedPtr.h>

namespace valkyrie::kernel {

template <typename T>
class WeakPtr {
  // Friend declaration
  template <typename U>
  friend class WeakPtr;

  template <typename U>
  friend class SharedPtr;

  friend struct Hash<WeakPtr<T>>;

 public:
  // Default constructor
  WeakPtr() : _ctrl() {}

  // Constructor (from a SharedPtr<T>)
  explicit WeakPtr(const SharedPtr<T> &r) : _ctrl(r._ctrl) {
    inc_use_count_weak();
  }

  // Destructor
  ~WeakPtr() {
    dec_use_count_weak();
  }

  // Copy constructor
  WeakPtr(const WeakPtr &r) : _ctrl(r._ctrl) {
    inc_use_count_weak();
  }

  // Copy assignment operator
  WeakPtr &operator=(const WeakPtr &r) {
    if (_ctrl && _ctrl->use_count_weak > 0) {
      dec_use_count_weak();
    }

    _ctrl = r._ctrl;
    inc_use_count_weak();
    return *this;
  }

  // Move constructor
  WeakPtr(WeakPtr &&r) : _ctrl(r._ctrl) {
    r._ctrl = nullptr;
  }

  // Move assignment operator
  WeakPtr &operator=(WeakPtr &&r) {
    if (_ctrl && _ctrl->use_count_weak > 0) {
      dec_use_count_weak();
    }

    _ctrl = r._ctrl;
    r._ctrl = nullptr;
    return *this;
  }

  // Conversion operator
  operator SharedPtr<T>() const {
    return lock();
  }

  void reset() {
    dec_use_count_weak();
  }

  void swap(WeakPtr &r) {
    using ::valkyrie::kernel::swap;
    swap(*this, r);
  }

  int use_count() const {
    return (_ctrl) ? _ctrl->use_count : 0;
  }

  bool expired() const {
    return use_count() == 0;
  }

  SharedPtr<T> lock() const {
    return expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
  }

 protected:
  void inc_use_count_weak() {
    if (_ctrl) {
      _ctrl->use_count_weak++;
    }
  }

  void dec_use_count_weak() {
    if (_ctrl && _ctrl->use_count_weak > 0) {
      _ctrl->use_count_weak--;
      maybe_delete_ctrl_block();
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

  typename SharedPtr<T>::ControlBlock *_ctrl;
};

// Explicit (full) specialization of struct `Hash` for WeakPtr<T>
template <typename T>
struct Hash<WeakPtr<T>> {
  size_t operator()(const WeakPtr<T> &wp) const {
    constexpr size_t prime = 31;
    size_t ret = 7;

    ret += prime * hash(wp._ctrl);
    return ret;
  }
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_WEAK_PTR_H_

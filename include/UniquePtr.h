// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UNIQUE_PTR_H_
#define VALKYRIE_UNIQUE_PTR_H_

namespace valkyrie::kernel {

template <typename T>
class UniquePtr {
 public:
  UniquePtr() : _ptr() {}

  UniquePtr(T* ptr) : _ptr(ptr) {}

  ~UniquePtr() {
    delete _ptr;
  }

  UniquePtr(UniquePtr&& other) noexcept {

  }

  UniquePtr& operator=(UniquePtr&& other) noexcept {

  }

  UniquePtr(const UniquePtr&) = delete;
  UniquePtr& operator=(const UniquePtr&) = delete;


  T* operator ->() const {
    return get();
  }

  T* get() const {
    return _ptr;
  }

  void reset() {
    delete _ptr;
    _ptr = nullptr;
  }

  T* release() {
    T* ret = _ptr;
    return ret;
  }

 private:
  T* _ptr;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UNIQUE_PTR_H_

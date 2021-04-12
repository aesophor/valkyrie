// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_FUNCTIONAL_H_
#define VALKYRIE_FUNCTIONAL_H_

#include <Memory.h>

namespace valkyrie::kernel {

template <typename>
class Function;

// Partial specialization of class `Function`
template <typename ReturnType, typename... Args>
class Function<ReturnType(Args...)> {
 public:
  Function() : _callable() {}
  ~Function() = default;

  // Constructor from aribtrary type T where
  // T::operator() is defined.
  template <typename T>
  Function(const T& t) {
    *this = t;
  }

  template <typename T>
  Function& operator =(const T& t) {
    _callable = make_shared<CallableImpl<T>>(t);
    return *this;
  }

  // Copy constructor
  Function(const Function& other) {
    *this = other;
  }

  // Copy assignment operator
  Function& operator =(const Function& other) {
    _callable = other._callable;
    return *this;
  }

  // Move constructor
  Function(Function&& other) noexcept {
    *this = move(other);
  }

  // Move assignment operator
  Function& operator =(Function&& other) noexcept {
    _callable = move(other._callable);
    return *this;
  }


  ReturnType operator ()(Args... args) const {
    return _callable->call(args...);
  }

  operator bool() const { return _callable; }


 private:
  class Callable {
   public:
    virtual ~Callable() = default;
    virtual ReturnType call(Args... args) = 0;
  };

  template <typename T>
  class CallableImpl : public Callable {
   public:
    explicit CallableImpl(const T& t) : _t(t) {}
    virtual ~CallableImpl() = default;

    virtual ReturnType call(Args... args) override {
      return _t(args...);
    }

   private:
    T _t;
  };

  SharedPtr<Callable> _callable;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FUNCTIONAL_H_

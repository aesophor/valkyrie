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

  // Constructor
  template <typename T>
  Function(const T& t)
      : _callable(make_unique<CallableImpl<T>>(t)) {}

  // Copy assignment operator
  template <typename T>
  Function& operator =(const T& t) {
    _callable = make_unique<CallableImpl<T>>(t);
    return *this;
  }

  ReturnType operator ()(Args... args) const {
    return _callable->invoke(args...);
  }

  operator bool() const { return _callable; }


  class Callable {
   public:
    virtual ~Callable() = default;
    virtual ReturnType invoke(Args... args) = 0;
  };

  template <typename T>
  class CallableImpl : public Callable {
   public:
    explicit CallableImpl(const T& t) : _t(t) {}
    virtual ~CallableImpl() = default;

    virtual ReturnType invoke(Args... args) override {
      return _t(args...);
    }

   private:
    T _t;
  };

 private:
  UniquePtr<Callable> _callable;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FUNCTIONAL_H_

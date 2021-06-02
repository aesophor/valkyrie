// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_FUNCTIONAL_H_
#define VALKYRIE_FUNCTIONAL_H_

#include <Concepts.h>
#include <Memory.h>

namespace valkyrie::kernel {

template <typename>
class Function;


// Partial specialization of class `Function`
template <typename ReturnType, typename... Args>
class Function<ReturnType(Args...)> {
 public:
  // Default constructor
  Function() : _callable() {}

  // Destructor
  ~Function() = default;

  // Constructor from aribtrary type T where
  // T::operator() is defined.
  template <typename T>
  Function(T t)
      : _callable(make_shared<CallableImpl<T>>(t)) {}
  
  template <typename T>
  Function& operator =(T t) {
    _callable = make_shared<CallableImpl<T>>(t);
    return *this;
  }

  // Copy constructor
  Function(const Function& r)
      : _callable(r._callable) {}

  // Copy assignment operator
  Function& operator =(const Function& r) {
    _callable = r._callable;
    return *this;
  }

  // Move constructor
  Function(Function&& r) noexcept
      : _callable(move(r._callable)) {}

  // Move assignment operator
  Function& operator =(Function&& r) noexcept {
    _callable = move(r._callable);
    return *this;
  }


  ReturnType operator ()(Args... args) const {
    // Check if `_callable` is nullptr.
    // If it is, then we'll print a warning message
    // and there should be a data abort exception, which
    // causes the kernel to panic.
    return _callable->call(args...);
  }

  operator bool() const {
    return _callable;
  }


 private:
  class CallableIface {
   public:
    virtual ~CallableIface() = default;
    virtual ReturnType call(Args... args) = 0;
  };

  template <typename T>
  class CallableImpl final : public CallableIface {
   public:
    explicit CallableImpl(const T& t) : _t(t) {}
    virtual ~CallableImpl() = default;

    virtual ReturnType call(Args... args) override {
      return _t(args...);
    }

   private:
    T _t;
  };

  SharedPtr<CallableIface> _callable;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FUNCTIONAL_H_

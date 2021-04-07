// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_STRING_H_
#define VALKYRIE_STRING_H_

#include <UniquePtr.h>

namespace valkyrie::kernel {

class String {
 public:
  // Default constructor
  String() : _s() {}

  // Constructor
  explicit
  String(char* s) : _s(s) {}

  // Destructor
  ~String() = default;

  // Copy constructor
  String(const String&) {}

  // Copy assignment operator
  String& operator =(const String& other) {
    return *this;
  }
  
  // Move constructor
  String(String&& other) noexcept {

  }
 
  // Move assignment operator
  String& operator =(String&& other) noexcept {
    return *this;
  }

 private:
  UniquePtr<char[]> _s;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_STRING_H_

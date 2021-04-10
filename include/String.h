// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_STRING_H_
#define VALKYRIE_STRING_H_

#include <UniquePtr.h>
#include <libs/CString.h>

namespace valkyrie::kernel {

class String {
 public:
  // Default constructor
  String() : _s() {}

  // Constructor
  String(const char* s)
      : _s(make_unique<char[]>(strlen(s) + 1)) {
    strcpy(_s.get(), s);
  }

  // Destructor
  ~String() = default;

  // Copy constructor
  String(const String& other) {
    *this = other;
  }

  // Copy assignment operator
  String& operator =(const String& other) {
    _s = make_unique<char[]>(other.size() + 1);
    strcpy(_s.get(), other._s.get());
    return *this;
  }
  
  // Move constructor
  String(String&& other) {
    *this = move(other);
  }
 
  // Move assignment operator
  String& operator =(String&& other) noexcept {
    _s = move(other._s);
    return *this;
  }


  char& operator [](size_t i) { return _s[i]; }

  String operator +(const String& other) const {
    String ret;
    ret._s = make_unique<char[]>(size() + other.size() + 1);
    strcpy(ret._s.get(), _s.get());
    strcat(ret._s.get(), other._s.get());
    return ret;
  }


  size_t size() const { return (_s) ? strlen(_s.get()) : 0; }

  const char* c_str() const { return _s.get(); }

 private:
  UniquePtr<char[]> _s;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_STRING_H_

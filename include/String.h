// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_STRING_H_
#define VALKYRIE_STRING_H_

#include <Iterator.h>
#include <Memory.h>
#include <libs/CString.h>

namespace valkyrie::kernel {

class String {
 public:
  using ValueType = char;
  using ConstIterator = BasicIterator<const String, const ValueType>;
  using Iterator = BasicIterator<String, ValueType>;

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
  String(const String& r) {
    *this = r;
  }

  // Copy assignment operator
  String& operator =(const String& r) {
    _s = make_unique<char[]>(r.size() + 1);
    strcpy(_s.get(), r._s.get());
    return *this;
  }
  
  // Move constructor
  String(String&& r) noexcept : _s(move(r._s)) {}
 
  // Move assignment operator
  String& operator =(String&& r) noexcept {
    _s = move(r._s);
    return *this;
  }


  char& operator [](size_t i) { return _s[i]; }

  bool operator ==(const String& other) const {
    return !strcmp(c_str(), other.c_str());
  }

  String operator +(const String& other) const {
    String ret;
    ret._s = make_unique<char[]>(size() + other.size() + 1);
    strcpy(ret._s.get(), _s.get());
    strcat(ret._s.get(), other._s.get());
    return ret;
  }


  Iterator begin() { return Iterator::begin(*this); }
  Iterator end() { return Iterator::end(*this); }
  ConstIterator begin() const { return ConstIterator::begin(*this); }
  ConstIterator end() const { return ConstIterator::end(*this); }


  size_t size() const { return (_s) ? strlen(_s.get()) : 0; }
  const char* c_str() const { return _s.get(); }

 private:
  UniquePtr<char[]> _s;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_STRING_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_STRING_H_
#define VALKYRIE_STRING_H_

#include <Iterator.h>
#include <Hash.h>
#include <List.h>
#include <Memory.h>
#include <libs/CString.h>

namespace valkyrie::kernel {

class String {
 public:
  using ValueType = char;
  using ConstIterator = ContiguousIterator<const String, const ValueType>;
  using Iterator = ContiguousIterator<String, ValueType>;

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
    *this = r;  // delegate to copy assignment operator
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
  const char& operator [](size_t i) const { return _s[i]; }

  bool operator ==(const String& r) const {
    return !strcmp(c_str(), r.c_str());
  }

  String operator +(const String& r) const {
    if (r.empty()) {
      return *this;
    }

    String ret;
    ret._s = make_unique<char[]>(size() + r.size() + 1);
    strcpy(ret._s.get(), _s.get());
    strcat(ret._s.get(), r._s.get());
    return ret;
  }

  String& operator +=(const String& r) {
    return *this = move(*this + r);
  }

  //operator bool() const { return !empty(); }


  Iterator begin() { return Iterator::begin(*this); }
  Iterator end() { return Iterator::end(*this); }
  ConstIterator begin() const { return ConstIterator::begin(*this); }
  ConstIterator end() const { return ConstIterator::end(*this); }


  // Searches the string for the first character that matches any of
  // the characters specified in its arguments.
  //
  // When pos is specified, the search only includes characters at
  // or after position pos, ignoring any possible occurrences before pos.
  size_t find_first_of(char c, size_t pos = 0) const {
    size_t len = size();
    for (size_t i = 0; i < len; i++) {
      if (_s[i] == c) {
        return i;
      }
    }
    return npos;
  }

  size_t find_first_not_of(char c, size_t pos = 0) const {
    size_t len = size();
    for (size_t i = 0; i < len; i++) {
      if (_s[i] != c) {
        return i;
      }
    }
    return npos;
  }

  // Searches the string for the last character that matches any of
  // the characters specified in its arguments.
  //
  // When pos is specified, the search only includes characters at
  // or before position pos, ignoring any possible occurrences after pos.
  size_t find_last_of(char c, size_t pos = npos) const {
    if (empty()) {
      return npos;
    }

    pos = (pos == npos) ? size() - 1 : pos;
    for (int i = pos; i >= 0; i--) {
      if (_s[i] == c) {
        return i;
      }
    }
    return npos;
  }

  size_t find_last_not_of(char c, size_t pos = npos) const {
    if (empty()) {
      return npos;
    }

    pos = (pos == npos) ? size() - 1 : pos;
    for (int i = pos; i >= 0; i--) {
      if (_s[i] != c) {
        return i;
      }
    }
    return npos;
  }


  // FIXME: maybe we should conform to the STL version...
  // but I'm too busy this week @_@
  void remove(const char val) {
    size_t slow = 0;
    size_t fast = 0;
    
    while (fast < size()) {
      if (_s[fast] == val) {
        fast++;
      } else {
        _s[slow] = _s[fast];
        slow++;
        fast++;
      }
    }
    
    size_t new_size = size() - (fast - slow);
    if (new_size > 0) {
      _s[new_size - 1] = 0;
    } else {
      _s[0] = 0;
    }
  }


  String substr(size_t begin, size_t len = npos) const {
    if (empty()) {
      return {};
    }

    // Sanitize `len`.
    if (len == npos || begin + len > size()) {
      len = size() - begin;
    }

    String ret;
    ret._s = make_unique<char[]>(len + 1);
    strncpy(ret._s.get(), _s.get() + begin, len);
    return ret;
  }

  // Splits this string by `delimiter` into at most `limit` substrings,
  // and places the substrings into a List.
  List<String> split(const char delimiter = ' ', size_t limit = 0) const {
    if (empty()) {
      return {};
    }

    List<String> substrings;
    size_t begin = 0;
    for (size_t i = 0; i < size() && substrings.size() != limit - 1; i++) {
      char c = _s[i];
      if (c == delimiter) {
        size_t len = i - begin;
        if (len) {
          substrings.push_back(substr(begin, len));
        }
        begin = i + 1;
      }
    }

    size_t tail_len = size() - begin;
    if (tail_len) {
      substrings.push_back(substr(begin, tail_len));
    }

    return substrings;
  }

  // Returns a string in which the string elements of sequence `seq`
  // have been joined by *this.
  // FIXME: pass by const ref (currently the ConstIterator is broken)
  [[nodiscard]]
  String join(List<String>& seq) {
    const size_t len = seq.size();
    String ret;

    for (auto it = seq.begin(); it != seq.end(); it++) {
      ret += *it;

      if (it.index() != len - 1) [[likely]] {
        ret += *this;
      }
    }

    return ret;
  }

  void to_upper() {
    constexpr int offset = 'a' - 'A';

    for (auto& c : *this) {
      if (c >= 'a' && c <= 'z') {
        c -= offset;
      }
    }
  }

  void to_lower() {
    constexpr int offset = 'a' - 'A';

    for (auto& c : *this) {
      if (c >= 'A' && c <= 'Z') {
        c += offset;
      }
    }
  }


  void clear() { _s.reset(); }
  bool empty() const { return size() == 0; }
  size_t size() const { return (_s) ? strlen(_s.get()) : 0; }
  const char& at(size_t i) const { return _s[i]; }
  const char* c_str() const { return _s.get(); }

  char& front() { return _s[0]; }
  char& back() { return _s[size() - 1]; }
  const char& front() const { return _s[0]; }
  const char& back() const { return _s[size() - 1]; }


  // Until the end of the string.
  static const size_t npos = -1;

 private:
  UniquePtr<char[]> _s;
};


// Explicit (full) specialization of `struct Hash` for String.
template <>
struct Hash<String> final {
  size_t operator ()(const String& s) const {
    constexpr size_t prime = 19;
    size_t ret = 5;

    for (auto c : s) {
      ret += prime * hash(c);
    }
    return ret;
  }
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_STRING_H_

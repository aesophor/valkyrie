// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ALGORITHM_H_
#define VALKYRIE_ALGORITHM_H_

namespace valkyrie::kernel {

template <typename T>
const T& min(const T& a, const T& b) {
  return (a <= b) ? a : b;
}

template <typename T>
const T& max(const T& a, const T& b) {
  return (a >= b) ? a : b;
}

inline bool is_digit(const char c) {
  return c >= '0' && c <= '9';
}

template <typename Iterator, typename Predicate>
Iterator find_if(Iterator begin, Iterator end, Predicate predicate) {
  while (begin != end && !static_cast<bool>(predicate(*begin))) {
    ++begin;
  }
  return begin;
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ALGORITHM_H_

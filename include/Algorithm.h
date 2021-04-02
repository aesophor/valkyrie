// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ALGORITHM_H_
#define VALKYRIE_ALGORITHM_H_

namespace valkyrie::kernel {

template <class T>
const T& min(const T& a, const T& b) {
  return (a <= b) ? a : b;
}

template <class T>
const T& max(const T& a, const T& b) {
  return (a >= b) ? a : b;
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ALGORITHM_H_

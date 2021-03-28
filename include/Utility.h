// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UTILITY_H_
#define VALKYRIE_UTILITY_H_

namespace valkyrie::kernel {

template <typename T1, typename T2>
struct Pair {
  T1 first;
  T2 second;
};

template <typename T>
void swap(T& t1, T& t2) {
  T temp = t1;
  t1 = t2;
  t2 = temp;
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UTILITY_H_

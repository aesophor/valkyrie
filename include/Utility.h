// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_UTILITY_H_
#define VALKYRIE_UTILITY_H_

#include <Types.h>

namespace valkyrie::kernel {

template <typename T1, typename T2>
struct Pair {
  T1 first;
  T2 second;
};


template<class T>
struct _RemoveExtent { using Type = T; };
 
template<class T>
struct _RemoveExtent<T[]> { using Type = T; };
 
template<class T, size_t N>
struct _RemoveExtent<T[N]> { using Type = T; };

template< class T >
using RemoveExtent = typename _RemoveExtent<T>::Type;


template <typename T>
struct _RemoveReference { using Type = T; };

template <typename T>
struct _RemoveReference<T&> { using Type = T; };

template <typename T>
struct _RemoveReference<T&&> { using Type = T; };

template <typename T>
using RemoveReference = typename _RemoveReference<T>::Type;


template <typename T>
constexpr RemoveReference<T>&& move(T&& t) {
  return static_cast<RemoveReference<T>&&>(t);
}

template <typename T>
constexpr T&& forward(RemoveReference<T>& t) {
  return static_cast<T&&>(t);
}

template <typename T>
constexpr T&& forward(RemoveReference<T>&& t) {
  return static_cast<T&&>(t);
}


template <typename T>
constexpr void swap(T& t1, T& t2) {
  T tmp = move(t1);
  t1 = move(t2);
  t2 = move(tmp);
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_UTILITY_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TYPES_H_
#define VALKYRIE_TYPES_H_

#define INT_MAX  2147483647
#define INT_MIN -2147483648

using uint64_t = unsigned long long;
using int64_t = long long;

using uint32_t = unsigned int;
using int32_t = int;

using uint16_t = unsigned short;
using int16_t = short;

using uint8_t = unsigned char;
using int8_t = char;

using size_t = unsigned long;

using pid_t = uint32_t;



// Template metaprogramming... here we go...
namespace valkyrie::kernel {

template <typename T>
struct _RemoveExtent { using Type = T; };

template <typename T>
struct _RemoveExtent<T[]> { using Type = T; };

template <typename T, size_t N>
struct _RemoveExtent<T[N]> { using Type = T; };

template <typename T >
using RemoveExtent = typename _RemoveExtent<T>::Type;


template <typename T>
struct _RemoveReference { using Type = T; };

template <typename T>
struct _RemoveReference<T&> { using Type = T; };

template <typename T>
struct _RemoveReference<T&&> { using Type = T; };

template <typename T>
using RemoveReference = typename _RemoveReference<T>::Type;

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TYPES_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TYPE_TRAITS_H_
#define VALKYRIE_TYPE_TRAITS_H_

#include <Types.h>

#define MAKE_NONCOPYABLE(C) \
  C(const C &) = delete;    \
  C &operator=(const C &) = delete

#define MAKE_NONMOVABLE(C) \
  C(C &&) = delete;        \
  C &operator=(C &&) = delete

// Template metaprogramming... Here we go.
// clang-format off
namespace valkyrie::kernel {

template <typename T> struct _RemoveExtent { using Type = T; };
template <typename T> struct _RemoveExtent<T[]> { using Type = T; };
template <typename T, size_t N> struct _RemoveExtent<T[N]> { using Type = T; };
template <typename T> struct _RemoveReference { using Type = T; };
template <typename T> struct _RemoveReference<T &> { using Type = T; };
template <typename T> struct _RemoveReference<T &&> { using Type = T; };

template <typename T> struct _RemoveCv { using Type = T; };
template <typename T> struct _RemoveCv<const T> { using Type = T; };
template <typename T> struct _RemoveCv<volatile T> { using Type = T; };
template <typename T> struct _RemoveCv<const volatile T> { using Type = T; };
template <typename T> struct _RemoveConst { using Type = T; };
template <typename T> struct _RemoveConst<const T> { using Type = T; };
template <typename T> struct _RemoveVolatile { using Type = T; };
template <typename T> struct _RemoveVolatile<volatile T> { using Type = T; };

template <typename T> using RemoveExtent = typename _RemoveExtent<T>::Type;
template <typename T> using RemoveReference = typename _RemoveReference<T>::Type;
template <typename T> using RemoveCv = typename _RemoveCv<T>::Type;
template <typename T> using RemoveConst = typename _RemoveConst<T>::Type;
template <typename T> using RemoveVolatile = typename _RemoveVolatile<T>::Type;

template <class T, T v>
struct _IntegralConstant {
  static constexpr T value = v;
  using value_type = T;
  using type = _IntegralConstant;  // using injected-class-name

  constexpr operator value_type() const noexcept {
    return value;
  }

  constexpr value_type operator()() const noexcept {
    return value;
  }
};

using TrueType = _IntegralConstant<bool, true>;
using FalseType = _IntegralConstant<bool, false>;


template <typename T, typename U> struct _IsSame : FalseType {};
template <typename T> struct _IsSame<T, T> : TrueType {};

template <typename T, typename U> inline constexpr bool IsSame = _IsSame<T, U>::value;


template <typename T> struct _IsPointer : FalseType {};
template <typename T> struct _IsPointer<T*> : TrueType {};
template <typename T> struct _IsPointer<T* const> : TrueType {};
template <typename T> struct _IsPointer<T* volatile> : TrueType {};
template <typename T> struct _IsPointer<T* const volatile> : TrueType {};

template <typename T> inline constexpr bool IsPointer = _IsPointer<T>::value;


template <typename> struct IsIntegralBase : FalseType {};

#define DEFINE_INTEGRAL_TYPE(type) \
  template <> struct IsIntegralBase<type> : TrueType {}

DEFINE_INTEGRAL_TYPE(bool);
DEFINE_INTEGRAL_TYPE(uint64_t);
DEFINE_INTEGRAL_TYPE(int64_t);
DEFINE_INTEGRAL_TYPE(uint32_t);
DEFINE_INTEGRAL_TYPE(int32_t);
DEFINE_INTEGRAL_TYPE(uint16_t);
DEFINE_INTEGRAL_TYPE(int16_t);
DEFINE_INTEGRAL_TYPE(uint8_t);
DEFINE_INTEGRAL_TYPE(int8_t);
DEFINE_INTEGRAL_TYPE(size_t);

template <typename T> struct _IsIntegral : IsIntegralBase<RemoveCv<T>> {};

template <typename T > inline constexpr bool IsIntegral = _IsIntegral<T>::value;


namespace internal {

template <typename B> TrueType test_pre_ptr_convertible(const volatile B *);
template <typename> FalseType test_pre_ptr_convertible(const volatile void *);

template <typename, typename>
auto test_pre_is_base_of(...) -> TrueType;

template <typename B, typename D>
auto test_pre_is_base_of(int)
    -> decltype(test_pre_ptr_convertible<B>(static_cast<D *>(nullptr)));

}  // namespace internal

template <typename Base, typename Derived>
struct IsBaseOf
    : _IntegralConstant<bool,
                        // IsClass<Base>::value && IsClass<Derived>::value &&
                        decltype(internal::test_pre_is_base_of<Base, Derived>(0))::value> {};

}  // namespace valkyrie::kernel

// clang-format on

#endif  // VALKYRIE_TYPE_TRAITS_H_

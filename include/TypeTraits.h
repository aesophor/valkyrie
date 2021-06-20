// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TYPE_TRAITS_H_
#define VALKYRIE_TYPE_TRAITS_H_

#include <Types.h>

#define MAKE_NONCOPYABLE(C)        \
 private:                          \
  C(const C&) = delete;            \
  C& operator =(const C&) = delete

#define MAKE_NONMOVABLE(C)         \
 private:                          \
  C(C&&) = delete;                 \
  C& operator =(C&&) = delete


// Template metaprogramming... Here we go.
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





template<class T, T v>
struct _IntegralConstant {
  static constexpr T value = v;
  using value_type = T;
  using type = _IntegralConstant;  // using injected-class-name
  constexpr operator value_type() const noexcept { return value; }
  constexpr value_type operator ()() const noexcept { return value; }
};

using TrueType = _IntegralConstant<bool, true>;
using FalseType = _IntegralConstant<bool, false>;


namespace internal {

template <typename B>
TrueType test_pre_ptr_convertible(const volatile B*);

template <typename>
FalseType test_pre_ptr_convertible(const volatile void*);
 
template <typename, typename>
auto test_pre_is_base_of(...) -> TrueType;

template <typename B, typename D>
auto test_pre_is_base_of(int) ->
    decltype(test_pre_ptr_convertible<B>(static_cast<D*>(nullptr)));

}  // namespace internal


template <typename Base, typename Derived>
struct IsBaseOf :
    _IntegralConstant<
        bool,
        //IsClass<Base>::value && IsClass<Derived>::value &&
        decltype(internal::test_pre_is_base_of<Base, Derived>(0))::value
    > { };

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TYPE_TRAITS_H_

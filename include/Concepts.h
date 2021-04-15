// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONCEPTS_H_
#define VALKYRIE_CONCEPTS_H_

#include <Utility.h>

namespace valkyrie::kernel {

template <typename T>
concept Callable = requires (T t) { t(); };


template <typename T>
concept EqualityComparible = requires (T a, T b) { a == b; };

template <typename T>
concept InequalityComparible = requires (T a, T b) { a != b; };

template <typename T>
concept Incrementible = requires (T t) {
  ++t;
  t++;
  *t++;
};

template <typename T>
concept Decrementible = requires (T t) {
  --t;
  t--;
  *t--;
};


template <typename T>
concept DereferencibleAsRvalue = requires (T&& t) {
  *move(t);
   move(t).operator->();
};

template <typename T, typename U>
concept DereferencibleAsLvalue = requires (T t, U u) {
  *t = u;
  *t++ = u;
};

template <typename T>
concept DefaultConstructible = requires (T t) {
  T();
  t = T();
};

/*
template <typename T>
concept CopyConstructible = requires (T t1, const T& t2) {

};

template <typename T>
concept CopyAssignable = requires (T t) {

};

template <typename T>
concept Destructible = requires (T t) {

};
*/

// Multi-pass: neither dereferencing nor incrementing affects dereferenceability
template <typename T>
concept Multipass = requires (T a, T b) {
  b = a; *a++; *b;
};


// There are six types of iterators in modern C++.
// In C++20, we can implement them using concepts.
// see: https://www.cplusplus.com/reference/iterator/
template <typename T>
concept InputIterator = EqualityComparible<T> &&
                        InequalityComparible<T> &&
                        DereferencibleAsRvalue<T>;

template <typename T, typename U>
concept OutputIterator = InputIterator<T> &&
                         DereferencibleAsLvalue<T, U>; // FIXME: not sure

/*
template <typename T>
concept ForwardIterator = InputIterator<T> &&
                          OutputIterator<T> &&
                          DefaultConstructible<T>;
*/

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CONCEPTS_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONCEPTS_H_
#define VALKYRIE_CONCEPTS_H_

#include <TypeTraits.h>

namespace valkyrie::kernel {

template <typename T>
concept Integral = IsIntegral<T>;

template <typename T>
concept Pointer = IsPointer<T>;

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CONCEPTS_H_

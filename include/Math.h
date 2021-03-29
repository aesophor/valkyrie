// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MATH_H_
#define VALKYRIE_MATH_H_

#include <Types.h>

namespace valkyrie::kernel {

size_t round_up_to_pow2(size_t x);
int pow(int base, int exponent);
int sqrt(int x);
int log2(int x);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MATH_H_

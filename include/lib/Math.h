// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MATH_H_
#define VALKYRIE_MATH_H_

#include <Types.h>

namespace valkyrie::kernel {

int pow(int base, int exponent);
int sqrt(int x);
int log2(int x);

size_t round_up_to_pow_of_2(size_t x);
size_t round_up_to_multiple_of_n(size_t x, const size_t n);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MATH_H_

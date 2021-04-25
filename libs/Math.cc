// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <libs/Math.h>

namespace valkyrie::kernel {

int pow(int base, int exponent) {
  int result = 1;
  while (exponent-- > 0) {
    result *= base;
  }
  return result;
}

int sqrt(int x) {
  int l = 1;
  int r = x;

  while (l <= r) {
    int mid = l + (r - l) / 2;

    // 2 == 5 / 2  --> for 5, we want to get 2
    if (mid == x / mid) {
      return mid;
    } else if (mid < x / mid) {
      l = mid + 1;
    } else {
      r = mid - 1;
    }
  }
  return l - 1;
}

int log2(int x) {
  int result = 0;
  while (x >>= 1) {
    ++result;
  }
  return result;
}

size_t round_up_to_pow_of_2(size_t x) {
  size_t result = 1;
  while (result < x) {
    result <<= 1;
  }
  return result;
}

size_t round_up_to_multiple_of_n(size_t x, const size_t n) {
  size_t result = n;
  while (result < x) {
    result += n;
  }
  return result;
}

}  // namespace valkyrie::kernel

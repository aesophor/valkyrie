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

}  // namespace valkyrie::kernel

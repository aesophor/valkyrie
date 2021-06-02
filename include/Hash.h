// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_HASH_H_
#define VALKYRIE_HASH_H_

#include <Types.h>

namespace valkyrie::kernel {

template <typename Key>
struct Hash;

// Explicit (full) specialization of struct `Hash` for primitive types.
#define DEFINE_TRIVIAL_HASH(key_type)         \
  template <>                                 \
  struct Hash<key_type> {                     \
    size_t operator ()(key_type key) const {  \
      return static_cast<size_t>(key);        \
    }                                         \
  }

DEFINE_TRIVIAL_HASH(bool);
DEFINE_TRIVIAL_HASH(char);
DEFINE_TRIVIAL_HASH(signed char);
DEFINE_TRIVIAL_HASH(unsigned char);
DEFINE_TRIVIAL_HASH(short);
DEFINE_TRIVIAL_HASH(int);
DEFINE_TRIVIAL_HASH(long);
DEFINE_TRIVIAL_HASH(long long);
DEFINE_TRIVIAL_HASH(unsigned short);
DEFINE_TRIVIAL_HASH(unsigned int);
DEFINE_TRIVIAL_HASH(unsigned long);
DEFINE_TRIVIAL_HASH(unsigned long long);

// TODO: add full specialization for float and double.


}  // namespace valkyrie::kernel

#endif  // VALKYRIE_HASH_H_

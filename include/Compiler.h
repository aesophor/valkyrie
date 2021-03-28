// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_COMPILER_H_
#define VALKYRIE_COMPILER_H_

#define likely(condition)   __builtin_expect(static_cast<bool>(condition), true)
#define unlikely(condition) __builtin_expect(static_cast<bool>(condition), false)

#endif  // VALKYRIE_COMPILER_H_

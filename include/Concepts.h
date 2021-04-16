// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CONCEPTS_H_
#define VALKYRIE_CONCEPTS_H_

namespace valkyrie::kernel {

template <typename T>
concept Callable = requires (T t) { t(); };

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CONCEPTS_H_

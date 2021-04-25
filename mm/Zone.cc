// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/Zone.h>

namespace valkyrie::kernel {

Zone::Zone(const size_t begin_addr, const size_t size)
    : _begin_addr(begin_addr),
      _size(size),
      _buddy_allocator(begin_addr),
      _slob_allocator(&_buddy_allocator) {}

}  // namespace valkyrie::kernel

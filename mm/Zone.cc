// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/Zone.h>

namespace valkyrie::kernel {

Zone::Zone(const size_t begin_addr, const size_t size)
    : begin_addr(begin_addr),
      size(size),
      buddy_allocator(begin_addr),
      slob_allocator(&buddy_allocator) {}

}  // namespace valkyrie::kernel

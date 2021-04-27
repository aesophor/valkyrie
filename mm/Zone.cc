// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/Zone.h>

namespace valkyrie::kernel {

Zone::Zone(const size_t begin_addr)
    : buddy_allocator(begin_addr),
      slob_allocator(&buddy_allocator) {}


size_t Zone::get_pages_count() {
  // Defined in include/mm/BuddyAllocator.h
  return MAX_ORDER_NR_PAGES;
}

}  // namespace valkyrie::kernel

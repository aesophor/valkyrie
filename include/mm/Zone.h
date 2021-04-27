// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// Currently, the definition of "Zone" in valkyrie is simple.
//
// Each zone is managed by exactly one buddy allocator,
// and on top of that there's a dynamic allocator which manages
// small memory chunks.

#ifndef VALKYRIE_ZONE_H_
#define VALKYRIE_ZONE_H_

#include <mm/BuddyAllocator.h>
#include <mm/SlobAllocator.h>

namespace valkyrie::kernel {

struct Zone final {
  explicit Zone(const size_t begin_addr);

  // Returns the number of pages in each zone.
  static size_t get_pages_count();

  BuddyAllocator buddy_allocator;
  SlobAllocator  slob_allocator;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ZONE_H_

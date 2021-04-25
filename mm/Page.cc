// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/Page.h>

#include <libs/CString.h>
#include <mm/BuddyAllocator.h>

namespace valkyrie::kernel {

Page::Page(void* user_data_addr)
    : _user_data_addr(user_data_addr) {}


void Page::copy_from(const Page& source) {
  memcpy(get(),
         source.get(),
         PAGE_SIZE - BuddyAllocator::get_block_header_size());
}


size_t Page::begin() const {
  return reinterpret_cast<size_t>(_user_data_addr) -
         BuddyAllocator::get_block_header_size();
}

size_t Page::data() const {
  return reinterpret_cast<size_t>(_user_data_addr);
}

size_t Page::end() const {
  return begin() + PAGE_SIZE;
}

}  // namespace valkyrie::kernel

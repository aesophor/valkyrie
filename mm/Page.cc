// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/Page.h>

#include <libs/CString.h>
#include <mm/PageFrameAllocator.h>

namespace valkyrie::kernel {

void Page::copy_from(const Page& source) {
  memcpy(_user_data_addr,
         source.get(),
         PAGE_SIZE - PageFrameAllocator::get_block_header_size());
}


size_t Page::begin() const {
  return reinterpret_cast<size_t>(_user_data_addr) -
         PageFrameAllocator::get_block_header_size();
}

size_t Page::data() const {
  return reinterpret_cast<size_t>(_user_data_addr);
}

size_t Page::end() const {
  return begin() + PAGE_SIZE;
}

}

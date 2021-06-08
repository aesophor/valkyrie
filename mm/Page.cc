// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/Page.h>

#include <libs/CString.h>

namespace valkyrie::kernel {

Page::Page(void* p) : _p(p) {}


void Page::copy_from(const Page& source) {
  memcpy(get(), source.get(), PAGE_SIZE);
}


size_t Page::begin() const {
  return reinterpret_cast<size_t>(_p);
}

size_t Page::data() const {
  return reinterpret_cast<size_t>(_p);
}

size_t Page::end() const {
  return begin() + PAGE_SIZE;
}


void* Page::get() const {
  return _p;
}

}  // namespace valkyrie::kernel

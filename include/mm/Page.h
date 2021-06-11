// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PAGE_H_
#define VALKYRIE_PAGE_H_

#include <Types.h>
#include <libs/CString.h>
#include <mm/BuddyAllocator.h>

#define PAGE_SIZE 4096

namespace valkyrie::kernel {

class Page {
 public:
  // Constructor
  Page(void* p_addr,
       void* v_addr = nullptr)
      : _p_addr(p_addr),
        _v_addr(v_addr) {}

  // Destructpr
  virtual ~Page() = default;


  // Copies the user data only, will not touch the header.
  void copy_from(const Page& source) {
    memcpy(_p_addr, source._p_addr, PAGE_SIZE);
  }

  // Call memset on this page, setting all bytes to 0x00.
  void clear() const {
    memset(_p_addr, 0, PAGE_SIZE);
  }

  // Returns the offset of `addr` relative to `_user_data_addr`.
  template <typename T, typename ReturnType = size_t>
  ReturnType offset_of(T addr) const {
    // Use C-style typecasting here since we are not sure
    // whether we should use static_cast or reinterpret_cast.
    size_t ret = ((size_t) addr) - begin();
    return reinterpret_cast<ReturnType>(ret);
  }

  template <typename ReturnType = size_t>
  ReturnType add_offset(size_t offset) const {
    size_t ret = begin() + reinterpret_cast<size_t>(offset);
    return reinterpret_cast<ReturnType>(ret);
  }

  size_t begin() const { return reinterpret_cast<size_t>(_p_addr); }
  size_t end() const { return begin() + PAGE_SIZE; }
  void* p_addr() const { return _p_addr; }
  void* v_addr() const { return _v_addr; }

  void set_v_addr(void* v_addr) { _v_addr = v_addr; }
  void set_p_addr(void* p_addr) { _p_addr = p_addr; }

 protected:
  void* _p_addr;
  void* _v_addr;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PAGE_H_

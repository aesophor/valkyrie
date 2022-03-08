// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PAGE_H_
#define VALKYRIE_PAGE_H_

#include <Concepts.h>
#include <CString.h>

#include <mm/mmu.h>
#include <mm/BuddyAllocator.h>

namespace valkyrie::kernel {

class Page {
 public:
  // Constructor
  Page(void *p_addr, void *v_addr = nullptr) : _p_addr(p_addr), _v_addr(v_addr) {}

  // Destructpr
  virtual ~Page() = default;

  // Copies the user data only, will not touch the header.
  void copy_from(const Page &source) {
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

  size_t begin() const {
    return reinterpret_cast<size_t>(_p_addr);
  }

  size_t end() const {
    return begin() + PAGE_SIZE;
  }

  void *p_addr() const {
    return _p_addr;
  }

  void *v_addr() const {
    return _v_addr;
  }

  void set_v_addr(void *v_addr) {
    _v_addr = v_addr;
  }

  void set_p_addr(void *p_addr) {
    _p_addr = p_addr;
  }

  // Is the given address aligned to a page boundary?
  template <typename T>
  requires IsIntegral<T> || IsPointer<T>
  static bool is_aligned(T addr) {
    return to_size_t(addr) % PAGE_SIZE == 0;
  }

  // Rounds down `addr` to the current page boundary.
  template <typename T>
  requires IsIntegral<T> || IsPointer<T>
  static T align_down(T addr) {
    return to_size_t(addr) & PAGE_MASK;
  }

  // Rounds up `addr` to the next page boundary.
  template <typename T>
  requires IsIntegral<T> || IsPointer<T>
  static constexpr T align_up(T addr) {
    return (to_size_t(addr) + (PAGE_SIZE - 1)) & PAGE_MASK;
  }

 private:
  template <Integral T>
  static size_t to_size_t(T addr) {
    return addr;
  }

  template <Pointer T>
  static size_t to_size_t(T addr) {
    return reinterpret_cast<size_t>(addr);
  }

  void *_p_addr;
  void *_v_addr;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PAGE_H_

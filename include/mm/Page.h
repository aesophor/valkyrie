// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PAGE_H_
#define VALKYRIE_PAGE_H_

#include <Types.h>
#include <mm/BuddyAllocator.h>

#define PAGE_SIZE        4096
#define PAGE_HEADER_SIZE (BuddyAllocator::get_block_header_size())
#define PAGE_DATA_SIZE   (PAGE_SIZE - PAGE_HEADER_SIZE)

namespace valkyrie::kernel {

class Page final {
 public:
  // Constructor
  explicit
  Page(void* user_data_addr);


  // Copies the user data only, will not touch the header.
  void copy_from(const Page& source);

  // Returns the offset of `addr` relative to `_user_data_addr`.
  template <typename T, typename ReturnType = size_t>
  ReturnType offset_of(T addr) const {
    // Use C-style typecasting here since we are not sure
    // whether we should use static_cast or reinterpret_cast.
    size_t ret = ((size_t) addr) - data();
    return reinterpret_cast<ReturnType>(ret);
  }

  template <typename ReturnType = size_t>
  ReturnType add_offset(size_t offset) const {
    size_t ret = data() + reinterpret_cast<size_t>(offset);
    return reinterpret_cast<ReturnType>(ret);
  }

  size_t begin() const;
  size_t data() const;
  size_t end() const;
  void* get() const;

 private:
  void* _user_data_addr;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PAGE_H_

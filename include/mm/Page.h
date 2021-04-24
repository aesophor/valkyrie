// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PAGE_H_
#define VALKYRIE_PAGE_H_

#include <Types.h>

#define PAGE_SIZE 4096

namespace valkyrie::kernel {

class Page final {
 public:
  // Constructor
  explicit
  Page(void* user_data_addr) : _user_data_addr(user_data_addr) {} 

  // Destructor
  ~Page() = default;


  // Copies the user data only, will not touch the header.
  void copy_from(const Page& source);

  // Returns the offset of `addr` relative to `_user_data_addr`.
  template <typename T, typename ReturnType = size_t>
  ReturnType offset_of(T addr) const {
    // Use C-style typecasting here since we are no sure
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

  void* get() const { return _user_data_addr; }

 private:
  void* _user_data_addr;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PAGE_H_

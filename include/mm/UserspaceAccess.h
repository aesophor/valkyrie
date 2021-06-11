// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_USERSPACE_ACCESS_H_
#define VALKYRIE_USERSPACE_ACCESS_H_

#include <proc/Task.h>

// Marks a pointer as a userspace pointer,
// indicating that we shouldn't simply dereference it.
#define __user

namespace valkyrie::kernel {

// Converts the virtual address to physical address
// by parsing the page table of current process.
template <typename T>
T copy_from_user(const void __user* const p) {
  return reinterpret_cast<T>(
      Task::current()->get_vmmap().get_physical_address(p));
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_USERSPACE_ACCESS_H_

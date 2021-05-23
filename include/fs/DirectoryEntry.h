// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// DirectoryEntry.h - Holds basic info of a dentry which can be passed from
//                    the kernel space to user space.

#ifndef VALKYRIE_DIRECTORY_ENTRY_H_
#define VALKYRIE_DIRECTORY_ENTRY_H_

#define NAME_LEN 256

namespace valkyrie::kernel {

struct DirectoryEntry {
  char name[NAME_LEN];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_DIRECTORY_ENTRY_H_

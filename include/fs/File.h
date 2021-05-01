// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// File.h - Represents a file opened by the kernel.

#ifndef VALKYRIE_FILE_H_
#define VALKYRIE_FILE_H_

#include <Types.h>

namespace valkyrie::kernel {

// Forward declaration
class Inode;


struct File final {
  File(Inode* inode, int flags) : _inode(inode), _flags(flags) {}

  Inode* _inode;
  size_t _pos;  // the next r/w position of this opened file.
  int _flags;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FILE_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// File.h - Represents a file opened by the kernel.
//
// Reference:
// [1] https://man7.org/training/download/lusp_fileio_slides.pdf

#ifndef VALKYRIE_FILE_H_
#define VALKYRIE_FILE_H_

#include <Types.h>

namespace valkyrie::kernel {

// Forward declaration
class FileSystem;
class Vnode;


struct File final {
  File(FileSystem& fs, Vnode* inode, int flags)
      : fs(fs),
        vnode(inode),
        pos(0),
        flags(flags) {}

  FileSystem& fs;  // the filesystem to which this file belong.
  Vnode* vnode;
  size_t pos;  // the next r/w position of this opened file.
  int flags;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FILE_H_

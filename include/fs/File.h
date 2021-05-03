// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// File.h - Represents a file opened by the kernel.
//
// Reference:
// [1] https://man7.org/training/download/lusp_fileio_slides.pdf

#ifndef VALKYRIE_FILE_H_
#define VALKYRIE_FILE_H_

#include <Types.h>
#include <Memory.h>
#include <fs/Vnode.h>

namespace valkyrie::kernel {

// Forward declaration
class FileSystem;
class Vnode;


struct File final {
  File(FileSystem& fs, SharedPtr<Vnode> vnode, int options)
      : fs(fs),
        vnode(move(vnode)),
        pos(),
        options(options) {}

  FileSystem& fs;  // the filesystem to which this file belong.
  SharedPtr<Vnode> vnode;
  size_t pos;  // the next r/w position of this opened file.
  int options;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FILE_H_

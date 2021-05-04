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
  File(FileSystem& fs, SharedPtr<Vnode> vnode, int options);

  FileSystem& fs;  // the filesystem to which this file belong.
  SharedPtr<Vnode> vnode;
  size_t pos;  // the next r/w position of this opened file.
  int options;


  // Some file descriptors such as 0, 1 and 2
  // are reserved and opened by default, but they don't really
  // require a meaningful open file structure, so we'll create
  // a "dummy" file structure here... which indicates that
  // the target fd is opened.
  //
  // For its usage, see fs/FileDescriptorTable.cc
  static const SharedPtr<File> opened;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FILE_H_

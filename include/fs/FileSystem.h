// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// FileSystem.h - The abstract base class of all concrete fs classes.

#ifndef VALKYRIE_FILE_SYSTEM_H_
#define VALKYRIE_FILE_SYSTEM_H_

#include <String.h>
#include <fs/File.h>

namespace valkyrie::kernel {

class FileSystem {
 public:
  virtual ~FileSystem() = default;

  virtual void show() const = 0;
  virtual SharedPtr<Vnode> get_root_vnode() = 0;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FILE_SYSTEM_H_

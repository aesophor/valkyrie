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

  virtual SharedPtr<Vnode> create(const String& pathname,
                                  const char* content,
                                  size_t size,
                                  mode_t mode,
                                  uid_t uid,
                                  gid_t gid) = 0;

  virtual SharedPtr<File> open(const String& pathname, int flags) = 0;
  virtual int close(SharedPtr<File> file) = 0;
  virtual int write(SharedPtr<File> file, const void* buf, size_t len) = 0;
  virtual int read(SharedPtr<File> file, void* buf, size_t len) = 0;

  virtual void show() const = 0;
  virtual SharedPtr<Vnode> get_root_vnode() = 0;
  virtual SharedPtr<Vnode> get_vnode(const String& pathname) = 0;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FILE_SYSTEM_H_

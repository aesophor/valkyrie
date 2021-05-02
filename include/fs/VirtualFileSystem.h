// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_VIRTUAL_FILE_SYSTEM_H_
#define VALKYRIE_VIRTUAL_FILE_SYSTEM_H_

#include <List.h>
#include <Memory.h>
#include <fs/FileSystem.h>

namespace valkyrie::kernel {

class VirtualFileSystem final {
 public:
  struct Mount final {
    UniquePtr<FileSystem> fs;
  };

  static VirtualFileSystem& get_instance();
  ~VirtualFileSystem() = default;

  bool mount_rootfs(UniquePtr<FileSystem> fs);

  VirtualFileSystem::Mount& get_rootfs();

 private:
  VirtualFileSystem();

  Mount _rootfs;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VIRTUAL_FILE_SYSTEM_H_

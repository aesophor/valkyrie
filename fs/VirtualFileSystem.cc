// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/VirtualFileSystem.h>

namespace valkyrie::kernel {

VirtualFileSystem& VirtualFileSystem::get_instance() {
  static VirtualFileSystem instance;
  return instance;
}


VirtualFileSystem::VirtualFileSystem()
    : _rootfs() {}


bool VirtualFileSystem::mount_rootfs(UniquePtr<FileSystem> fs) {
  _rootfs = {move(fs)};
  // TODO: maybe add some error checking here...
  return _rootfs.fs;
}


VirtualFileSystem::Mount& VirtualFileSystem::get_rootfs() {
  return _rootfs;
}

}  // namespace valkyrie::kernel

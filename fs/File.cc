// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/File.h>

#include <fs/VirtualFileSystem.h>

namespace valkyrie::kernel {

const SharedPtr<File> File::opened
  = make_shared<File>(*VirtualFileSystem::get_instance().get_rootfs().fs, nullptr, 0);


File::File(FileSystem& fs, SharedPtr<Vnode> vnode, int options)
    : fs(fs),
      vnode(move(vnode)),
      pos(),
      options(options) {}

}  // namespace valkyrie::kernel

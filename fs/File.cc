// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/File.h>

namespace valkyrie::kernel {

File::File(FileSystem& fs, SharedPtr<Vnode> vnode, int options)
    : fs(fs),
      vnode(move(vnode)),
      pos(),
      options(options) {}

}  // namespace valkyrie::kernel

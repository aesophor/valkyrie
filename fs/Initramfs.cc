// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/Initramfs.h>

namespace valkyrie::kernel {

Initramfs::Initramfs() : _archive(CPIO_ARCHIVE_ADDR) {
  _archive.parse();
}

}  // namespace valkyrie::kernel

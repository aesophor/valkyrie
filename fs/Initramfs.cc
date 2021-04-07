// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/Initramfs.h>

namespace valkyrie::kernel {

Initramfs::Initramfs() : _archive(CPIO_ARCHIVE_ADDR) {}


const char* Initramfs::read(const char* pathname, size_t* size) const {
  return _archive.get_entry_content(pathname, size);
}

}  // namespace valkyrie::kernel

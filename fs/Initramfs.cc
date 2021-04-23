// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/Initramfs.h>

namespace valkyrie::kernel {

Initramfs& Initramfs::get_instance() {
  static Initramfs instance;
  return instance;
}

Initramfs::Initramfs() : _archive(CPIO_ARCHIVE_ADDR) {}


Pair<const char*, size_t> Initramfs::read(const char* name) const {
  return _archive.get_entry_content_and_size(name);
}

}  // namespace valkyrie::kernel

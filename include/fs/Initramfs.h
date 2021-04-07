// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_INITRAMFS_H_
#define VALKYRIE_INITRAMFS_H_

#include <fs/CPIOArchive.h>

namespace valkyrie::kernel {

class Initramfs {
 public:
  Initramfs();
  ~Initramfs() = default;

  const char* read(const char* pathname, size_t* size) const;

 private:
  CPIOArchive _archive;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CPIO_H_

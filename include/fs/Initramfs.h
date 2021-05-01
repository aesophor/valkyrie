// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_INITRAMFS_H_
#define VALKYRIE_INITRAMFS_H_

#include <Utility.h>
#include <fs/CPIOArchive.h>

namespace valkyrie::kernel {

class Initramfs {
 public:
  static Initramfs& get_instance();
  ~Initramfs() = default;

  Pair<const char*, size_t> read(const char* name) const;
  CPIOArchive _archive;

 private:
  Initramfs();

};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CPIO_H_

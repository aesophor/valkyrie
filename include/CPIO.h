// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CPIO_H_
#define VALKYRIE_CPIO_H_

#include <Types.h>

#define CPIO_BASE 0x8000000

namespace valkyrie::kernel {

class CPIO {
 public:
  CPIO(char* ptr);
  ~CPIO() = default;

 private:
  struct [[gnu::packed]] Header final {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
  };

  struct DirectoryEntry final {
    DirectoryEntry(const char* ptr);

    const CPIO::Header* header;
    const char* pathname;
    const char* content;
    size_t pathname_len;
    size_t content_len;
  };
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CPIO_H_

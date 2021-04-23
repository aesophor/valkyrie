// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CPIO_ARCHIVE_H_
#define VALKYRIE_CPIO_ARCHIVE_H_

#include <Types.h>
#include <Utility.h>

#define CPIO_ARCHIVE_ADDR 0x8000000

namespace valkyrie::kernel {

class CPIOArchive {
 public:
  explicit CPIOArchive(const size_t base_addr);
  ~CPIOArchive() = default;

  Pair<const char*, size_t>
  get_entry_content_and_size(const char* name) const;

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
    DirectoryEntry() = default;
    DirectoryEntry(const char* ptr);

    operator bool() const;

    const CPIOArchive::Header* header;
    const char* pathname;
    const char* content;
    size_t pathname_len;
    size_t content_len;
  };

  const char* _base_addr;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CPIO_ARCHIVE_H_

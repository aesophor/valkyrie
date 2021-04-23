// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/CPIOArchive.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <libs/CString.h>

#define CPIO_MAGIC     "070701"
#define CPIO_MAGIC_LEN 6
#define CPIO_TRAILER   "TRAILER!!!"

namespace valkyrie::kernel {

CPIOArchive::CPIOArchive(const size_t base_addr)
    : _base_addr(reinterpret_cast<const char*>(base_addr)) {}


Pair<const char*, size_t>
CPIOArchive::get_entry_content_and_size(const char* name) const {
  const char* ptr = _base_addr;
  DirectoryEntry dentry;

  while ((dentry = DirectoryEntry(ptr))) {
    //printf("(%s)<%s> = %d\n", name, dentry.pathname, dentry.content_len);

    if (!strcmp(name, dentry.pathname)) {
      return {dentry.content, dentry.content_len};
    }

    // Advance ptr until it reaches the next header.
    ptr += sizeof(CPIOArchive::Header) + dentry.pathname_len + dentry.content_len;
    while (strncmp(ptr, CPIO_MAGIC, CPIO_MAGIC_LEN)) ++ptr;
  }

  return {nullptr, 0};
}


CPIOArchive::DirectoryEntry::DirectoryEntry(const char* ptr)
    : header(reinterpret_cast<const CPIOArchive::Header*>(ptr)) {
  char buf[16];

  // Obtain pathname size from the header.
  memset(buf, 0, sizeof(buf));
  strncpy(buf, header->c_namesize, 8);
  pathname_len = atoi(buf, 16);

  // Obtain content size from the header.
  memset(buf, 0, sizeof(buf));
  strncpy(buf, header->c_filesize, 8);
  content_len = atoi(buf, 16);

  // Update pathname and content pointers.
  pathname = (pathname_len) ? ptr + sizeof(CPIOArchive::Header) : 0;
  content = (content_len) ? ptr + sizeof(CPIOArchive::Header) + pathname_len : 0;

  // Advance content pointer.
  while (content_len && !*content) ++content;
}

CPIOArchive::DirectoryEntry::operator bool() const {
  return strcmp(pathname, CPIO_TRAILER);
}

}  // namespace valkyrie::kernel

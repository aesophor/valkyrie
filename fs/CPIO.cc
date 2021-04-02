// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/CPIO.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <libs/String.h>

#define CPIO_MAGIC     "070701"
#define CPIO_MAGIC_LEN 6
#define CPIO_TRAILER   "TRAILER!!!"

namespace valkyrie::kernel {

CPIO::CPIO(const size_t base_addr)
    : _base_addr(reinterpret_cast<const char*>(base_addr)) {}


void CPIO::parse() const {
  const char* ptr = _base_addr;
  DirectoryEntry dentry;

  while ((dentry = DirectoryEntry(ptr))) {
    printf("%s \t = %s\n", dentry.pathname, dentry.content);

    // Advance ptr until it reaches the next header.
    ptr += sizeof(CPIO::Header) + dentry.pathname_len + dentry.content_len;
    while (strncmp(ptr, CPIO_MAGIC, CPIO_MAGIC_LEN)) ++ptr;
  }
}


CPIO::DirectoryEntry::DirectoryEntry(const char* ptr)
    : header(reinterpret_cast<const CPIO::Header*>(ptr)) {
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
  pathname = (pathname_len) ? ptr + sizeof(CPIO::Header) : 0;
  content = (content_len) ? ptr + sizeof(CPIO::Header) + pathname_len : 0;

  // Advance content pointer.
  while (content_len && !*content) ++content;
}

CPIO::DirectoryEntry::operator bool() const {
  return strcmp(pathname, CPIO_TRAILER);
}

}  // namespace valkyrie::kernel

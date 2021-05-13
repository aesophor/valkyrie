// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/CPIOArchive.h>

#include <dev/Console.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/Kernel.h>
#include <libs/CString.h>

#define CPIO_MAGIC     "070701"
#define CPIO_TRAILER   "TRAILER!!!"

namespace valkyrie::kernel {

CPIOArchive::CPIOArchive(const size_t base_addr)
    : _base_addr(reinterpret_cast<const char*>(base_addr)),
      _ptr(_base_addr) {}


bool CPIOArchive::is_valid() const {
  return !strncmp(_base_addr, CPIO_MAGIC, sizeof(CPIO_MAGIC) - 1);
}

void CPIOArchive::for_each(Function<void (const CPIOArchive::Entry&)> callback) const {
  const char* ptr = _base_addr;

  while (true) {
    CPIOArchive::Entry dentry(ptr);

    if (!dentry.is_valid()) {
      break;
    }
  
    callback(dentry);

    // Advance `ptr` until it reaches the next header.
    ptr += sizeof(CPIOArchive::Header) + dentry.pathname_len + dentry.content_len;
    while (strncmp(ptr, CPIO_MAGIC, sizeof(CPIO_MAGIC) - 1)) ++ptr;
  }
}


CPIOArchive::Entry::Entry(const char* ptr)
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
  pathname = (pathname_len) ? ptr + sizeof(CPIOArchive::Header) : nullptr;
  content = (content_len) ? ptr + sizeof(CPIOArchive::Header) + pathname_len : nullptr;

  // Advance content pointer.
  while (content_len && !*content) ++content;
}

bool CPIOArchive::Entry::is_valid() const {
  return strncmp(pathname, CPIO_TRAILER, sizeof(CPIO_TRAILER) - 1);
}

}  // namespace valkyrie::kernel

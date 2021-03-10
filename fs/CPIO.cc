// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <CPIO.h>

#include <Console.h>
#include <String.h>

#define CPIO_MAGIC   "070701"
#define CPIO_TRAILER "TRAILER!!!"

namespace valkyrie::kernel {

CPIO::CPIO(const char* base_addr) : _base_addr(base_addr) {}


void CPIO::parse() const {
  for (const char* ptr = _base_addr; ;) {
    DirectoryEntry dentry = DirectoryEntry(ptr);

    if (!strcmp(dentry.pathname, CPIO_TRAILER)) {
      break;
    }

    printf("pathname     = %s\n", dentry.pathname);
    //printf("pathname_len = %d\n", dentry.pathname_len);
    if (dentry.content_len) {
      printf("content      = %s\n", dentry.content);
    }
    //printf("content_len  = %d\n", dentry.content_len);
    printf("-----\n");

    ptr += sizeof(CPIO::Header) + dentry.pathname_len + dentry.content_len;
    while (strncmp(ptr, CPIO_MAGIC, 6)) ++ptr;
  }
}


CPIO::DirectoryEntry::DirectoryEntry(const char* ptr)
    : header(reinterpret_cast<const CPIO::Header*>(ptr)) {
  char buf[16] = {0};
  strncpy(buf, header->c_namesize, 8);
  pathname_len = atoi(buf, 16);
  if (pathname_len % 2) {
    ++pathname_len;
  }

  memset(buf, 0, sizeof(buf));
  strncpy(buf, header->c_filesize, 8);
  content_len = atoi(buf, 16);
  if (content_len % 2) {
    ++content_len;
  }

  pathname = ptr + sizeof(CPIO::Header);
  content = ptr + sizeof(CPIO::Header) + pathname_len;

  if (content_len) {
    while (!*content) ++content;
  }
}

}  // namespace valkyrie::kernel

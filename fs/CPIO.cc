// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <CPIO.h>

#include <Console.h>
#include <String.h>

#define CPIO_MAGIC   "070701"
#define CPIO_TRAILER "TRAILER!!!"

namespace valkyrie::kernel {

CPIO::CPIO(char* ptr) {
  puts("[*] parsing initramfs...");

  CPIO::Header header;
  char pathname[64] = {0};
  char content[512] = {0};

  while (strcmp(content, CPIO_TRAILER)) {
    // Advance the pointer until the underlying byte != 0x00
    while (*ptr++ == 0);

    header = *reinterpret_cast<CPIO::Header*>(ptr);
    ptr += sizeof(CPIO::Header);

    size_t len = strlen(ptr);
    strcpy(pathname, ptr);
    ptr += len + 1;  // extra one for the null byte at the end

    char* content_begin = ptr;
    while (strcmp(ptr++, CPIO_MAGIC));
    memcpy(content, content_begin, ptr - content_begin);

    printf("pathname: %s\n", pathname);
    printf("content: %s\n", content);
    printf("------------------\n");
  }
}

}  // namespace valkyrie::kernel

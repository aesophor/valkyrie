// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/ELF.h>

#include <dev/Console.h>
#include <libs/CString.h>

#define ELF_MAGIC     "\x7f" "ELF"
#define ELF_MAGIC_LEN 4

namespace valkyrie::kernel {

ELF::ELF(const char* addr, const size_t size)
    : _header(reinterpret_cast<const ELF::Header*>(addr)) {
  if (memcmp(_header->e_ident, ELF_MAGIC, ELF_MAGIC_LEN)) {
    printf("Not ELF!!!\n");
    return;
  }

  printf("sizeof(ELF::Header) = %d\n", sizeof(Header));
}

}  // namespace valkyrie::kernel

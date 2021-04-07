// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/ELF.h>

#include <dev/Console.h>
#include <libs/CString.h>

#define ELF_MAGIC     "\x7f" "ELF"
#define ELF_MAGIC_LEN 4

namespace valkyrie::kernel {

ELF::ELF(const char* addr)
    : _header(reinterpret_cast<const ELF::Header*>(addr)),
      _entry_point(reinterpret_cast<void*>(_header->e_entry)) {
  if (memcmp(_header->e_ident, ELF_MAGIC, ELF_MAGIC_LEN)) {
    printf("Not ELF!!!\n");
    return;
  }

  printf("entry point: 0x%x\n", _header->e_entry);
}


void* ELF::get_entry_point() const {
  size_t elf_base = reinterpret_cast<size_t>(_header);
  size_t offset = sizeof(ELF::Header);
  return reinterpret_cast<void*>(elf_base + offset);
}

}  // namespace valkyrie::kernel

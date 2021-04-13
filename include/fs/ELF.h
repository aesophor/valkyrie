// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ELF_H_
#define VALKYRIE_ELF_H_

#include <Types.h>

#define EI_NIDENT (16)

namespace valkyrie::kernel {

class ELF {
 public:
  ELF(const char* addr, const size_t size);
  ~ELF() = default;

  struct [[gnu::packed]] Header final {
    unsigned char e_ident[EI_NIDENT];  // Magic number and other info
    uint16_t e_type;                   // Object file type
    uint16_t e_machine;                // Architecture
    uint32_t e_version;                // Object file version
    uint64_t e_entry;                  // Entry point virtual address
    uint64_t e_phoff;                  // Program header table file offset
    uint64_t e_shoff;                  // Section header table file offset
    uint32_t e_flags;                  // Processor-specific flags
    uint16_t e_ehsize;                 // ELF header size in bytes
    uint16_t e_phentsize;              // Program header table entry size
    uint16_t e_phnum;                  // Program header table entry count
    uint16_t e_shentsize;              // Section header table entry size
    uint16_t e_shnum;                  // Section header table entry count
    uint16_t e_shstrndx;               // Section header string table index
  };

 public:
  const Header* _header;
};


}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ELF_H_

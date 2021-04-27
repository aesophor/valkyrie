// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ELF_H_
#define VALKYRIE_ELF_H_

#include <Types.h>
#include <Utility.h>

#define EI_NIDENT (16)

namespace valkyrie::kernel {

class ELF {
 public:
  struct [[gnu::packed]] FileHeader final {
    unsigned char ident[EI_NIDENT];  // Magic number and other info
    uint16_t type;                   // Object file type
    uint16_t machine;                // Architecture
    uint32_t version;                // Object file version
    uint64_t entry;                  // Entry point virtual address
    uint64_t phoff;                  // Program header table file offset
    uint64_t shoff;                  // Section header table file offset
    uint32_t flags;                  // Processor-specific flags
    uint16_t ehsize;                 // ELF header size in bytes
    uint16_t phentsize;              // Program header table entry size
    uint16_t phnum;                  // Program header table entry count
    uint16_t shentsize;              // Section header table entry size
    uint16_t shnum;                  // Section header table entry count
    uint16_t shstrndx;               // Section header string table index
  };

  // An executable or shared object file's program header table is an
  // array of structures, each describing a segment or other
  // information the system needs to prepare the program for
  // execution.  An object file segment contains one or more sections.
  // Program headers are meaningful only for executable and shared
  // object files.  A file specifies its own program header size with
  // the ELF header's e_phentsize and e_phnum members.
  struct [[gnu::packed]] ProgramHeader final {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
  };

  // A file's section header table lets one locate all the file's sections.
  struct [[gnu::packed]] SectionHeader final {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entsize;
  };


  ELF(Pair<const char*, size_t> addr_size);
  ~ELF() = default;

  bool is_valid() const;
  void load_at(void* dest) const;
  void* get_entry_point(const void* elf_base) const;

  size_t get_size() const;

 private:
  const FileHeader* _header;

  const size_t _size;
};


}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ELF_H_

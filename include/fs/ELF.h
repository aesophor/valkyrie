// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// ELF.h - Executable and Linkable Format
//
// [1] https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab8.html#elf-loader
// [2] https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// [3] https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-83432/index.html
// [4] https://www.jollen.org/blog/2007/03/elf_program_loading_2_pht.html

#ifndef VALKYRIE_ELF_H_
#define VALKYRIE_ELF_H_

#include <Types.h>
#include <Utility.h>
#include <mm/VMMap.h>

// Default base address where an ELF will be loaded.
#define ELF_DEFAULT_BASE 0x400000

// ELF Segments Type
// https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-83432/index.html
#define PT_NULL               0
#define PT_LOAD               1
#define PT_DYNAMIC            2
#define PT_INTERP             3
#define PT_NOTE               4
#define PT_SHLIB              5
#define PT_PHDR               6
#define PT_LOSUNW    0x6ffffffa
#define PT_SUNWBSS   0x6ffffffb
#define PT_SUNWSTACK 0x6ffffffa
#define PT_HISUNW    0x6fffffff
#define PT_LOPROC    0x70000000
#define PT_HIPROC    0x7fffffff

// Legal values for p_flags (segment flags).
// https://www.jollen.org/blog/2007/03/elf_program_loading_2_pht.html
#define PF_X            (1 << 0)        /* Segment is executable */
#define PF_W            (1 << 1)        /* Segment is writable */
#define PF_R            (1 << 2)        /* Segment is readable */
#define PF_MASKOS       0x0ff00000      /* OS-specific */
#define PF_MASKPROC     0xf0000000      /* Processor-specific */

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

  bool is_valid() const;

  const char* get_raw_content() const;
  void load(const VMMap& vmmap) const;
  void* get_entry_point() const;

  size_t get_size() const;

 private:
  const FileHeader* _header;

  const size_t _size;
};


}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ELF_H_

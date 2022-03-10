// Copyright (c) 2021-2022 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// ELF.h - Executable and Linkable Format
//
// [1] https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab8.html#elf-loader
// [2] https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// [3] https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-83432/index.html
// [4] https://www.jollen.org/blog/2007/03/elf_program_loading_2_pht.html

#ifndef VALKYRIE_ELF_H_
#define VALKYRIE_ELF_H_

#include <CString.h>
#include <List.h>
#include <Memory.h>
#include <TypeTraits.h>
#include <Utility.h>

#include <fs/File.h>

// PIE elf base
#define ELF_DEFAULT_BASE 0x400000

namespace valkyrie::kernel {

class ELF {
  MAKE_NONCOPYABLE(ELF);

 public:
  static constexpr const size_t ident_len = 16;
  static constexpr const size_t magic_len = 4;
  static constexpr const char *magic = "\x7f\x45\x4c\x46";

  struct [[gnu::packed]] Header final {
    unsigned char ident[ident_len];  // Magic number and other info
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

  enum SegmentType {
    NULL,
    LOAD,
    DYNAMIC,
    INTERP,
    NOTE,
    SHLIB,
    PHDR,
    TLS,
    NUM,
    LOOS = 0x60000000,
    GNU_EH_FRAME = 0x6474e550,
    GNU_STACK = 0x6474e551,
    GNU_RELRO = 0x6474e552,
    GNU_PROPERTY = 0x6474e553,
    LOSUNW = 0x6ffffffa,
    SUNWBSS = 0x6ffffffb,
    SUNWSTACK = 0x6ffffffa,
    HISUNW = 0x6fffffff,
    HIOS = 0x6fffffff,
    LOPROC = 0x70000000,
    HIPROC = 0x7fffffff
  };

  struct Segment {
    // Segment flags (permissions).
    static constexpr uint32_t x = 1 << 0;
    static constexpr uint32_t w = 1 << 1;
    static constexpr uint32_t r = 1 << 2;

    // Returns the pointer to the raw data of this segment.
    const char *get_content(const char *elf_base) {
      return elf_base + file_offset;
    }

    ELF::SegmentType type;      // type
    uint32_t flags;             // flags
    uint64_t file_offset;       // offset
    uint64_t virtual_address;   // vaddr
    uint64_t physical_address;  // paddr
    uint64_t physical_size;     // filesz
    uint64_t virtual_size;      // memsz
    uint64_t alignment;         // align
  };

  // A file's section header table lets one locate all the file's sections.
  struct [[gnu::packed]] SectionHeader final {
    uint32_t name;       // Section name (string tbl index)
    uint32_t type;       // Section type
    uint64_t flags;      // Section flags
    uint64_t addr;       // Section virtual addr at execution
    uint64_t offset;     // Section file offset
    uint64_t size;       // Section size in bytes
    uint32_t link;       // Link to another section
    uint32_t info;       // Additional section information
    uint64_t addralign;  // Section alignment
    uint64_t entsize;    // Entry size if section holds table
  };


  ELF(SharedPtr<File> file)
      : _base(file ? file->vnode->get_content() : nullptr),
        _len(file ? file->vnode->get_size() : 0),
        _elf_header(file ? reinterpret_cast<const ELF::Header *>(_base) : nullptr),
        _segments() {
    if (!file) {
      return;
    }

    // Populate _segments by parsing the program headers.
    const char *ptr = _base + _elf_header->phoff;
    for (int i = 0; i < _elf_header->phnum; i++) {
      _segments.push_back(*reinterpret_cast<const Segment *>(ptr));
      ptr += _elf_header->phentsize;
    }
  }


  // TODO: Const correctness...
  List<Segment> &get_segments() {
    return _segments;
  }

  const char *get_base() const {
    return _base;
  }

  bool exists() const {
    return _base;
  }

  bool is_valid() const {
    return !memcmp(_elf_header->ident, ELF::magic, ELF::magic_len);
  }

  void *get_entry_point() const {
    return reinterpret_cast<void *>(ELF_DEFAULT_BASE + _elf_header->entry);
  }

 private:
  const char *_base;
  const size_t _len;

  const ELF::Header *_elf_header;
  List<Segment> _segments;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ELF_H_

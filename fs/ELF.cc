// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/ELF.h>

#include <dev/Console.h>
#include <libs/CString.h>

#define ELF_MAGIC     "\x7f" "ELF"
#define ELF_MAGIC_LEN 4

namespace valkyrie::kernel {

ELF::ELF(Pair<const char*, size_t> addr_size)
    : _header(reinterpret_cast<const ELF::FileHeader*>(addr_size.first)),
      _size(addr_size.second) {}


bool ELF::is_valid() const {
  return !memcmp(_header->ident, ELF_MAGIC, ELF_MAGIC_LEN);
}

void ELF::load_at(void* dest) const {
  //size_t p = reinterpret_cast<size_t>(dest);
  //size_t base = p;
  memcpy(dest, _header, _size);

  /*
  auto header = reinterpret_cast<const ELF::FileHeader*>(p);
  printk("loaded ELF at 0x%x\n", dest);
  printk("  <ELF file header> entry point: 0x%x\n", header->entry);
  printk("  <ELF file header> phentsize: 0x%x\n", header->phentsize);
  printk("  <ELF file header> phnum: 0x%x\n", header->phnum);
  printk("  <ELF file header> shentsize: 0x%x\n", header->shentsize);
  printk("  <ELF file header> shnum: 0x%x\n", header->shnum);
  p += header->shoff;

  for (int i = 0; i < header->shnum; i++) {
    auto s_header = reinterpret_cast<const ELF::SectionHeader*>(p);
    char* string_table = nullptr;

    if (s_header->type == 1) {  // SHT_PROGBITS
      printk("  <section header %d> name: %d\n", i,  s_header->name);
      printk("  <section header %d> offset: 0x%x\n", i, s_header->offset);
    } else if (s_header->type == 3) {  // SHT_STRTAB
      printk("  string table offset: 0x%x\n", s_header->offset);
      string_table = reinterpret_cast<char*>(base + s_header->offset);
      printf("%s\n", string_table + 27);
      printf("%s\n", string_table + 33);
      printf("%s\n", string_table + 46);
    }
    p += header->shentsize;
  }
  */
}

void* ELF::get_entry_point(const void* elf_base) const {
  // The implementation of this method is temporary
  // as virtual memory hasn't been enabled yet.
  size_t entry = reinterpret_cast<size_t>(elf_base) + 0x10000;
  return reinterpret_cast<void*>(entry);
}


size_t ELF::get_size() const {
  return _size;
}

}  // namespace valkyrie::kernel

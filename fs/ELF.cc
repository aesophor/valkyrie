// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/ELF.h>

#include <Algorithm.h>
#include <dev/Console.h>
#include <libs/CString.h>
#include <mm/MemoryManager.h>

#define ELF_MAGIC     "\x7f" "ELF"
#define ELF_MAGIC_LEN 4

namespace valkyrie::kernel {

ELF::ELF(Pair<const char*, size_t> addr_size)
    : _header(reinterpret_cast<const ELF::FileHeader*>(addr_size.first)),
      _size(addr_size.second) {}


bool ELF::is_valid() const {
  return !memcmp(_header->ident, ELF_MAGIC, ELF_MAGIC_LEN);
}

const char* ELF::get_raw_content() const {
  return reinterpret_cast<const char*>(_header);
}

void ELF::load(const VMMap& vmmap) const {
  auto p = reinterpret_cast<const char*>(_header);

  /*
  printk("loaded ELF at 0x%x\n", _header);
  printk("  <ELF file header> entry point: 0x%x\n", _header->entry);
  printk("  <ELF file header> phentsize: 0x%x\n", _header->phentsize);
  printk("  <ELF file header> phnum: 0x%x\n", _header->phnum);
  printk(" -----------------------------\n");
  */
  p += _header->phoff;

  for (int i = 0; i < _header->phnum; i++, p += _header->phentsize) {
    auto ph = reinterpret_cast<const ELF::ProgramHeader*>(p);

    if (ph->type != PT_LOAD) {
      continue;
    }

    /*
    printk("  <program header %d> type: %d\n", i,  ph->type);
    printk("  <program header %d> vaddr: 0x%x\n", i, ph->vaddr);
    printk("  <program header %d> offset: 0x%x\n", i, ph->offset);
    printk("  <program header %d> align: 0x%x\n", i, ph->align);
    printk("  <program header %d> filesz: 0x%x\n", i, ph->filesz);
    printk("  <program header %d> memsz: 0x%x\n", i, ph->memsz);
    printk("  <program header %d> flags: 0x%x\n", i, ph->flags);
    printk(" --------------------------------\n");
    */

    // `ph->filesz` represents the size of this segment,
    // so we need to loop until all contents are loaded.
    // source: https://wiki.osdev.org/ELF
    bool is_copying_first_page = true;
    size_t nr_pages = ph->filesz / PAGE_SIZE + static_cast<bool>(ph->filesz % PAGE_SIZE);
    //printk("this segment needs %d pages\n", nr_pages);

    for (size_t j = 0; j < nr_pages; j++) {
      size_t base = reinterpret_cast<size_t>(_header);
      size_t src = base + ph->offset + j * PAGE_SIZE;
      //printk("src = base 0x%x + ph->offset 0x%x + %x\n", base, ph->offset, j * PAGE_SIZE);
      void* src_p = reinterpret_cast<void*>(src);
      size_t len = PAGE_SIZE;

      void* page = get_free_page();
      size_t dest = reinterpret_cast<size_t>(page);

      if (is_copying_first_page) {
        dest += ph->offset & 0xfff;
        len = PAGE_SIZE - (ph->offset & 0xfff);
        is_copying_first_page = false;
      }

      void* dest_p = reinterpret_cast<void*>(dest);
      //printk("memcpy(0x%x, 0x%x, 0x%x)\n", dest_p, src_p, len);
      memcpy(dest_p, src_p, len);

      //printk("mapping page v: 0x%x <- p: 0x%x\n",
      //    ELF_DEFAULT_BASE + ph->vaddr + j * PAGE_SIZE,
      //    page);

      vmmap.map(ELF_DEFAULT_BASE + ph->vaddr + j * PAGE_SIZE,
                page,
                PAGE_RWX);
    }
  }
}

void* ELF::get_entry_point() const {
  return reinterpret_cast<void*>(ELF_DEFAULT_BASE + _header->entry);
}


size_t ELF::get_size() const {
  return _size;
}

}  // namespace valkyrie::kernel

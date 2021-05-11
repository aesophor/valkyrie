// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FAT32.h>

#include <dev/SDCardDriver.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

FAT32Vnode::FAT32Vnode(FAT32& fs,
                       FAT32Vnode* parent,
                       const String& name,
                       mode_t mode,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_vnode_index++, mode, uid, gid),
      _fs(fs),
      _name(name),
      _parent(parent),
      _children() {}


FAT32::FAT32()
    : _next_vnode_index(0),
      _root_vnode() {
  auto& sdcard_driver = SDCardDriver::get_instance();
  
  char buf[512];
  sdcard_driver.read_block(0, buf);
  for (auto& c : buf) {
    printf("%x ", c);
  }
  printf("\n");


  const MasterBootRecord* mbr = reinterpret_cast<MasterBootRecord*>(buf);

  if (mbr->signature != 0xaa55) [[unlikely]] {
    Kernel::panic("invalid boot sector.\n");
  }

  printk("disk id: %x\n", mbr->unique_disk_id);

  for (int i = 0; i < 4; i++) {
    printk("parition %d: ", i);

    // The partition table in MBR is not 16-byte aligned...
    // so we have to copy them. Also, we need a const reference
    // to the entry so that we can access the copied entry's bit fields...
    PartitionTableEntry _entry;
    const PartitionTableEntry& entry = _entry;
    memcpy(&_entry, &mbr->partitions[i], sizeof(PartitionTableEntry));

    printf("sector_idx = 0x%x\n", entry.lba_addr_partition_start);
  }

  printf("------\n");
  sdcard_driver.read_block(0x800, buf);
  for (auto& c : buf) {
    printf("%c ", c);
  }
  printf("\n");


  BootSector _bpb;
  const BootSector& bpb = _bpb;
  memcpy(&_bpb, buf, sizeof(BootSector));

  printk("reserved_sector_count: %d\n", bpb.reserved_sector_count);
  printk("root_cluster: 0x%x\n", bpb.root_cluster);

  // Access cluster #2
  int cluster_begin_lba = 0x800 + bpb.reserved_sector_count + (bpb.table_count * bpb.table_size_32);
  int idx = cluster_begin_lba + (2 - 2) * bpb.sectors_per_cluster;

  printf("------\n");
  sdcard_driver.read_block(idx, buf);
  for (auto& c : buf) {
    printf("%x ", c);
  }
  printf("\n");


  char* ptr = buf;
  DirectoryEntry dentry;

  while (true) {
    memcpy(&dentry, ptr, sizeof(DirectoryEntry));

    if (!dentry.name[0]) {
      break;
    }

    char name[16] = {0};
    char extension[16] = {0};
    strncpy(name, dentry.name, 8);
    strncpy(extension, dentry.extension, 3);
    printf("filename: %s, extension: %s\n", name, extension);

    ptr += 32;
  }
}


SharedPtr<Vnode> FAT32::get_root_vnode() {
  return _root_vnode;
}

}  // namespace valkyrie::kernel

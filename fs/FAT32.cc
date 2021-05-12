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
  printk("sectors_per_cluster: %d\n", bpb.sectors_per_cluster);
  printk("root_cluster: 0x%x\n", bpb.root_cluster);
  printk("table_size_32: 0x%x\n", bpb.table_size_32);

  // Print FAT #1
  /*
  for (int i = 0; i < bpb.table_size_32; i++) {
    int sector = 0x800 + bpb.reserved_sector_count + i;
    sdcard_driver.read_block(sector, buf);

    for (int j = 0; j < 512; j += 4) {
      uint32_t* p = reinterpret_cast<uint32_t*>(&buf[j]);
      printf("0x%x ", *p);
    }
    printf("---\n");
  }
  */


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
  ShortDirectoryEntry _dentry;

  while (true) {
    memcpy(&_dentry, ptr, sizeof(ShortDirectoryEntry));

    const ShortDirectoryEntry& dentry = _dentry;
    //printf("0x%x ", dentry.name[0]);

    if ((uint8_t) dentry.name[0] == 0xe5) {
      ptr += sizeof(ShortDirectoryEntry);
      continue;
    } else if (dentry.name[0] == 0) {
      break;
    }

    char name[16] = {0};
    char extension[16] = {0};
    strncpy(name, dentry.name, 8);
    strncpy(extension, dentry.extension, 3);
    printf("filename: %s, extension: %s cluster: [%d,%d], size = %d\n",
        name, extension, dentry.first_cluster_high, dentry.first_cluster_low, dentry.size);

    ptr += sizeof(ShortDirectoryEntry);
  }


  // Get file #1 content
  idx = cluster_begin_lba + (5865 - 2) * bpb.sectors_per_cluster;
  sdcard_driver.read_block(idx, buf);

  for (auto& c : buf) {
    printf("%c", c);
  }

  
  // Look up FAT 0
  // A single FAT contains multiple sectors,
  // so first thing first, we need to calculate the sector index.
  int sector_index = 0x800 + bpb.reserved_sector_count + (5865 / 128);
  printf("sector index = %d\n", sector_index);

  sdcard_driver.read_block(sector_index, buf);
  uint32_t* p = reinterpret_cast<uint32_t*>(buf);
  p += 5865 % 128;

  printf("next cluster number = %d\n", *p);



  /*
  // Get file #1 content
  idx = cluster_begin_lba + (5866 - 2) * bpb.sectors_per_cluster;
  sdcard_driver.read_block(idx, buf);

  for (auto& c : buf) {
    printf("%x", c);
  }
  */
 }


SharedPtr<Vnode> FAT32::get_root_vnode() {
  return _root_vnode;
}

}  // namespace valkyrie::kernel

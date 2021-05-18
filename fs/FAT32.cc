// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FAT32.h>

#include <Memory.h>
#include <dev/SDCardDriver.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

FAT32Inode::FAT32Inode(FAT32& fs,
                       FAT32Inode* parent,
                       const String& name,
                       mode_t mode,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_inode_index++, mode, uid, gid),
      _fs(fs),
      _name(name),
      _parent(parent),
      _children() {}


FAT32::FAT32(DiskPartition& disk_partition)
    : _disk_partition(disk_partition),
      _metadata(),
      _next_inode_index(0),
      _root_inode() {
  
  char buf[512];
  _disk_partition.read_block(0, buf);
  memcpy(&_metadata, buf, sizeof(BootSector));

  const BootSector& _metadata = this->_metadata;
  printk("reserved_sector_count: %d\n", _metadata.reserved_sector_count);
  printk("sectors_per_cluster: %d\n", _metadata.sectors_per_cluster);
  printk("root_cluster: 0x%x\n", _metadata.root_cluster);
  printk("table_size_32: 0x%x\n", _metadata.table_size_32);

  // Print FAT #1
  /*
  for (int i = 0; i < bpb.table_size_32; i++) {
    int sector = bpb.reserved_sector_count + i;
    _disk_partition.read_block(sector, buf);

    for (int j = 0; j < 512; j += 4) {
      uint32_t* p = reinterpret_cast<uint32_t*>(&buf[j]);
      printf("0x%x ", *p);
    }
    printf("---\n");
  }
  */


  // Access cluster #2
  int cluster_begin_lba = _metadata.reserved_sector_count +
                          (_metadata.table_count * _metadata.table_size_32);
  int idx = cluster_begin_lba + (2 - 2) * _metadata.sectors_per_cluster;

  printf("------\n");
  _disk_partition.read_block(idx, buf);
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
  idx = cluster_begin_lba + (5865 - 2) * _metadata.sectors_per_cluster;
  _disk_partition.read_block(idx, buf);

  for (auto& c : buf) {
    printf("%c", c);
  }

  
  // Look up FAT 0
  // A single FAT contains multiple sectors,
  // so first thing first, we need to calculate the sector index.
  int sector_index = _metadata.reserved_sector_count + (5866 / 128);
  printf("sector index = %d\n", sector_index);

  _disk_partition.read_block(sector_index, buf);
  uint32_t* p = reinterpret_cast<uint32_t*>(buf);
  p += 5866 % 128;

  printf("next cluster number = %x\n", *p);



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
  return _root_inode;
}

}  // namespace valkyrie::kernel

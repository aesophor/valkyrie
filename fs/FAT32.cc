// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FAT32.h>

#include <Memory.h>
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
      _metadata(disk_partition),
      _next_inode_index(0),
      _root_inode() {

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
  UniquePtr<char[]> cluster = cluster_read(2);
  char* ptr = cluster.get();
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
  cluster = cluster_read(5865);
  for (int i = 0; i < 512; i++) {
    printf("%c", cluster[i]);
  }

  // Look up FAT 0
  uint32_t next_cluster = file_allocation_table_read(5865);
  printf("next cluster number = %d\n", next_cluster);


  /*
  // Get file #1 content
  idx = cluster_begin_lba + (5866 - 2) * bpb.sectors_per_cluster;
  sdcard_driver.read_block(idx, buf);

  for (auto& c : buf) {
    printf("%x", c);
  }
  */
 }

uint32_t FAT32::file_allocation_table_read(const uint32_t cluster_number) const {
  // A single FAT contains multiple sectors,
  // so first thing first, we need to calculate the sector index.
  int index = _metadata.reserved_sector_count;
  index += cluster_number / (512 / 4);

  char buf[512];
  _disk_partition.read_block(index, buf);

  uint32_t* p = reinterpret_cast<uint32_t*>(buf);
  p += cluster_number % (512 / 4);
  return *p;
}

UniquePtr<char[]> FAT32::cluster_read(const uint32_t cluster_number) const {
  auto buffer = make_unique<char[]>(512);

  int index = _metadata.reserved_sector_count;
  index += _metadata.table_count * _metadata.table_size_32;
  index += (cluster_number - 2) * _metadata.sectors_per_cluster;
  
  _disk_partition.read_block(index, buffer.get());
  return buffer;
}


SharedPtr<Vnode> FAT32::get_root_vnode() {
  return _root_inode;
}


FAT32::BootSector::BootSector(DiskPartition& disk_partition) {
  char buf[512];
  disk_partition.read_block(0, buf);
  memcpy(this, buf, sizeof(BootSector));
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FAT32.h>

#include <Memory.h>
#include <kernel/Kernel.h>

#define FAT32_EOC_MIN   0x0ffffff8
#define FAT32_EOC_MAX   0x0fffffff
#define FAT32_IS_EOC(x) (FAT32_EOC_MIN <= x && x <= FAT32_EOC_MAX)

#define FAT32_MODE     0777

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

namespace valkyrie::kernel {

FAT32::FAT32(DiskPartition& disk_partition)
    : _disk_partition(disk_partition),
      _metadata(disk_partition),
      _next_inode_index(0),
      _root_inode(make_shared<FAT32Inode>(*this, "/", _metadata.root_cluster, 0, 0)) {

  _root_inode->for_each_child([](const auto& dentry) {
    printk("%s\n", dentry.get_filename().c_str());
  });
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



FAT32::ShortDirectoryEntry::ShortDirectoryEntry() {
  memset(this, 0, sizeof(FAT32::ShortDirectoryEntry));
}

FAT32::ShortDirectoryEntry::ShortDirectoryEntry(void* ptr) {
  memcpy(this, ptr, sizeof(FAT32::ShortDirectoryEntry));
}

String FAT32::ShortDirectoryEntry::get_filename() const {
  char buf[16];

  memset(buf, 0, sizeof(buf));
  strncpy(buf, name, sizeof(name));
  String filename = buf;
  filename = filename.substr(0, filename.find_last_not_of(' ') + 1);

  memset(buf, 0, sizeof(buf));
  strncpy(buf, extension, sizeof(extension));
  String ext = buf;
  ext = ext.substr(0, ext.find_last_not_of(' ') + 1);

  return (!ext.empty()) ? filename + "." + ext : filename;
}

uint32_t FAT32::ShortDirectoryEntry::get_cluster_number() const {
  uint32_t ret = first_cluster_high;
  return (ret << 16) | first_cluster_low;
}


FAT32::BootSector::BootSector(DiskPartition& disk_partition) {
  char buf[512];
  disk_partition.read_block(0, buf);
  memcpy(this, buf, sizeof(BootSector));
}



FAT32Inode::FAT32Inode(FAT32& fs,
                       const String& name,
                       uint32_t cluster_number,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_inode_index++, FAT32_MODE, uid, gid),
      _fs(fs),
      _name(name),
      _cluster_number(cluster_number),
      _content(make_unique<char[]>(512)) {}


SharedPtr<Vnode> FAT32Inode::get_child(const String& name) {
  auto dentry = find_child_if([&name](const auto& dentry) {
    return name == dentry.get_filename();
  });

  return (dentry) ? make_shared<FAT32Inode>(_fs, name, dentry.get_cluster_number(), 0, 0)
                  : nullptr;
}

SharedPtr<Vnode> FAT32Inode::get_ith_child(size_t i) {
  auto dentry = find_child_if([&i](const auto& dentry) {
    return i-- == 0;
  });

  return (dentry) ? make_shared<FAT32Inode>(_fs, dentry.get_filename(), dentry.get_cluster_number(), 0, 0)
                  : nullptr;
}

size_t FAT32Inode::get_children_count() const {

}

char* FAT32Inode::get_content() {
  _content = _fs.cluster_read(_cluster_number);
  return _content.get();

  // Get entire file's content
  /*
  for (uint32_t cluster_number = 5865;
       !FAT32_IS_EOC(cluster_number);
       cluster_number = file_allocation_table_read(cluster_number)) {
    cluster = cluster_read(5865);
    for (int i = 0; i < 512; i++) {
      printf("%c", cluster[i]);
    }
  }
  */
}


FAT32::ShortDirectoryEntry
FAT32Inode::find_child_if(Function<bool (const FAT32::ShortDirectoryEntry&)> predicate) {
  auto cluster = _fs.cluster_read(_cluster_number);

  for (char* ptr = cluster.get();; ptr += sizeof(FAT32::ShortDirectoryEntry)) {
    const FAT32::ShortDirectoryEntry dentry(ptr);

    if (dentry.is_deleted()) {
      continue;
    }

    if (dentry.is_end_of_cluster_chain()) {
      break;
    }

    if (predicate(dentry)) {
      return dentry;
    }
  }

  return {};
}

void FAT32Inode::for_each_child(Function<void (const FAT32::ShortDirectoryEntry&)> callback) {
  auto cluster = _fs.cluster_read(_cluster_number);

  for (char* ptr = cluster.get();; ptr += sizeof(FAT32::ShortDirectoryEntry)) {
    const FAT32::ShortDirectoryEntry dentry(ptr);

    if (dentry.is_deleted()) {
      continue;
    }

    if (dentry.is_end_of_cluster_chain()) {
      break;
    }

    callback(dentry);
  }
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FAT32.h>

#include <Memory.h>
#include <libs/Math.h>
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
      _nr_fat_entries_per_sector(_metadata.bytes_per_sector / sizeof(uint32_t)),
      _next_inode_index(0),
      _root_inode(make_shared<FAT32Inode>(*this, "/", _metadata.root_cluster, 0, S_IFDIR, 0, 0)) {

  _root_inode->for_each_child([](const auto& dentry) {
    printk("%s\n", dentry.get_filename().c_str());
  });
}


uint32_t FAT32::fat_read(const uint32_t i) const {
  if (i >= nr_single_fat_entries()) [[unlikely]] {
    printk("fat32: invalid fat entry_index (%d)\n", i);
    return FAT32_EOC_MAX;
  }

  char buf[512];
  _disk_partition.read_block(fat0_sector_index(/*entry_idx=*/i), buf);

  size_t offset = i % _nr_fat_entries_per_sector;  // offset within sector
  return *(reinterpret_cast<uint32_t*>(buf) + offset);
}

void FAT32::fat_write(const uint32_t i, const uint32_t val) const {
  if (i >= nr_single_fat_entries()) [[unlikely]] {
    printk("fat32: invalid fat entry_index (%d)\n", i);
    return;
  }

  char buf[512];
  _disk_partition.read_block(fat0_sector_index(/*entry_idx=*/i), buf);

  size_t offset = i % _nr_fat_entries_per_sector;  // offset within sector
  *(reinterpret_cast<uint32_t*>(buf) + offset) = val;
}


uint32_t FAT32::fat_find_free_cluster() const {
  for (auto index = fat0_sector_index(); index < fat1_sector_index(); index++) {
    char buf[512];
    _disk_partition.read_block(index, buf);

    uint32_t* p = reinterpret_cast<uint32_t*>(buf);
    for (int i = 0; i < _nr_fat_entries_per_sector; i++) {
      if (*p == 0) {
        printk("fat32: found a free cluster (id = %d)\n", index * 512 + i);
        return index * 512 + i;
      }
    }
  }

  printk("fat32: unable to find a free cluster in FAT. Is the storage full?\n");
  return -1;
}


void FAT32::cluster_read(const uint32_t i, void* buf) const {
  _disk_partition.read_block(cluster_sector_index(i), buf);
}

void FAT32::cluster_write(const uint32_t i, const void* buf) const {
  _disk_partition.write_block(cluster_sector_index(i), buf);
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
  filename.to_lower();

  memset(buf, 0, sizeof(buf));
  strncpy(buf, extension, sizeof(extension));
  String ext = buf;
  ext = ext.substr(0, ext.find_last_not_of(' ') + 1);
  ext.to_lower();

  return (!ext.empty()) ? filename + "." + ext : filename;
}

uint32_t FAT32::ShortDirectoryEntry::get_first_cluster_number() const {
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
                       uint32_t first_cluster_number,
                       off_t size,
                       mode_t mode,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_inode_index++, size, mode, uid, gid),
      _fs(fs),
      _name(name),
      _first_cluster_number(first_cluster_number),
      _content() {}


SharedPtr<Vnode> FAT32Inode::get_child(const String& name) {
  const auto dentry = find_child_if([&name](const auto& dentry) {
    return name == dentry.get_filename();
  });

  // FIXME: determine file type by attributes
  mode_t mode = (dentry.size) ? S_IFREG : S_IFDIR;

  return (dentry)
      ? make_shared<FAT32Inode>(_fs, name, dentry.get_first_cluster_number(), dentry.size, mode, 0, 0)
      : nullptr;
}

SharedPtr<Vnode> FAT32Inode::get_ith_child(size_t i) {
  const auto dentry = find_child_if([&i](const auto& dentry) {
    return i-- == 0;
  });

  mode_t mode = (dentry.size) ? S_IFREG : S_IFDIR;

  return (dentry)
      ? make_shared<FAT32Inode>(_fs, dentry.get_filename(), dentry.get_first_cluster_number(), dentry.size, mode, 0, 0)
      : nullptr;
}

size_t FAT32Inode::get_children_count() const {
  size_t ret = 0;

  for_each_child([&ret](const auto& dentry) {
    ++ret;
  });

  return ret;
}

char* FAT32Inode::get_content() {
  _content = make_unique<char[]>(round_up_to_multiple_of_n(_size, 512));

  int i = 0;
  for (uint32_t n = _first_cluster_number;
       !FAT32_IS_EOC(n);
       n = _fs.fat_read(n)) {
    _fs.cluster_read(n, _content.get() + 512 * i++);
  }

  return _content.get();
}

void FAT32Inode::set_content(UniquePtr<char[]> content, off_t new_size) {
  new_size = round_up_to_multiple_of_n(new_size, 512);

  uint32_t prev_free_cluster_number = _first_cluster_number;
  uint32_t curr_free_cluster_number = 0;

  for (uint32_t i = 0; i < new_size; i += 512) {
    if ((curr_free_cluster_number = _fs.fat_find_free_cluster()) == -1) {
      return;
    }

    // Write block to the data region
    _fs.cluster_write(curr_free_cluster_number, content.get() + i);

    // Update File Allocation Table
    if (prev_free_cluster_number) [[likely]] {
      _fs.fat_write(prev_free_cluster_number, curr_free_cluster_number);
    } else {
      _first_cluster_number = curr_free_cluster_number;
    }

    prev_free_cluster_number = curr_free_cluster_number;
  }

  _content = move(content);
  _size = new_size;

  // FIXME: Update dentry size in parent's cluster.

}


FAT32::ShortDirectoryEntry
FAT32Inode::find_child_if(Function<bool (const FAT32::ShortDirectoryEntry&)> predicate) {
  auto cluster = make_unique<char[]>(512);
  _fs.cluster_read(_first_cluster_number, cluster.get());

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

// FIXME: resolve this code duplication
void
FAT32Inode::for_each_child(Function<void (const FAT32::ShortDirectoryEntry&)> callback) const {
  auto cluster = make_unique<char[]>(512);
  _fs.cluster_read(_first_cluster_number, cluster.get());

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

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FAT32.h>

#include <List.h>
#include <Memory.h>
#include <libs/Math.h>
#include <kernel/Kernel.h>

#define FAT32_EOC_MIN   0x0ffffff8
#define FAT32_EOC_MAX   0x0fffffff
#define FAT32_IS_EOC(x) (FAT32_EOC_MIN <= x && x <= FAT32_EOC_MAX)

#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LFN_ENTRY (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

namespace valkyrie::kernel {

FAT32::FAT32(DiskPartition& disk_partition)
    : _disk_partition(disk_partition),
      _metadata(disk_partition),
      _nr_fat_entries_per_sector(_metadata.bytes_per_sector / sizeof(uint32_t)),
      _next_inode_index(0),
      _root_inode(make_shared<FAT32Inode>(*this, "/", _metadata.root_cluster, FAT32_EOC_MAX, 0, 0, S_IFDIR, 0, 0)) {}


uint32_t FAT32::fat_read(const uint32_t fat_entry_index) const {
  if (fat_entry_index >= nr_single_fat_entries()) [[unlikely]] {
    printk("fat32: invalid fat_entry_index (%d)\n", fat_entry_index);
    return FAT32_EOC_MAX;
  }

  auto buffer = make_unique<char[]>(_metadata.bytes_per_sector);
  size_t sector = fat0_sector_index(fat_entry_index);
  size_t offset = fat_entry_index % _nr_fat_entries_per_sector;  // offset within sector

  _disk_partition.read_block(sector, buffer.get());
  return *(reinterpret_cast<uint32_t*>(buffer.get()) + offset);
}

void FAT32::fat_write(const uint32_t fat_entry_index, const uint32_t val) const {
  if (fat_entry_index >= nr_single_fat_entries()) [[unlikely]] {
    printk("fat32: invalid fat entry_index (%d)\n", fat_entry_index);
    return;
  }

  auto buffer = make_unique<char[]>(_metadata.bytes_per_sector);
  size_t sector = fat0_sector_index(fat_entry_index);
  size_t offset = fat_entry_index % _nr_fat_entries_per_sector;  // offset within sector

  _disk_partition.read_block(sector, buffer.get());
  *(reinterpret_cast<uint32_t*>(buffer.get()) + offset) = val;

  _disk_partition.write_block(sector, buffer.get());
}


uint32_t FAT32::fat_find_free_cluster() const {
  for (uint32_t index = fat0_sector_index(); index < fat1_sector_index(); index++) {
    char buf[512];
    _disk_partition.read_block(index, buf);

    for (int i = 0; i < _nr_fat_entries_per_sector; i++) {
      if (*(reinterpret_cast<uint32_t*>(buf) + i) == 0) {
        return (index - fat0_sector_index()) * _nr_fat_entries_per_sector + i;
      }
    }
  }

  printk("fat32: unable to find a free cluster in FAT. Is the storage full?\n");
  return -1;
}


void FAT32::cluster_read(const uint32_t cluster_number, void* buf) const {
  _disk_partition.read_block(cluster_sector_index(cluster_number), buf);
}

void FAT32::cluster_write(const uint32_t cluster_number, const void* buf) const {
  _disk_partition.write_block(cluster_sector_index(cluster_number), buf);
}

String FAT32::generate_short_filename(String long_filename) const {
  char basis[8];
  char numeric_tail[3];

  // 1. The UNICODE name passed to the file system is converted to upper case.
  long_filename.to_upper();

  // 2. The upper cased UNICODE name is converted to OEM.
  bool lossy_conversion = false;
  for (auto& c : long_filename) {
    if (!is_valid_short_filename_char(c)) {
      c = '_';
      lossy_conversion = true;
    }
  }

  // 3. Strip all leading and embedded spaces from the long name.
  long_filename.remove(' ');

  // 4. Strip all leading periods from the long name.
  long_filename = long_filename.substr(long_filename.find_first_not_of('.'));

  // 5. (No desc)

}

bool FAT32::is_valid_short_filename_char(const uint8_t c) const {
  // According to: https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system
  // Legal characters for DOS short filenames include the following:
  // 1. Upper case letters A–Z
  // 2. Numbers 0–9
  // 3. Space
  // 4. ! # $ % & ' ( ) - @ ^ _ ` { } ~
  // 5. Characters 128–228
  // 6. Characters 230–255
  return (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') ||
         (c >= 128 && c <= 228) ||
         (c >= 230 && c <= 255) ||
         (c == ' ') ||
         (c == '!') ||
         (c == '#') ||
         (c == '$') ||
         (c == '%') ||
         (c == '&') ||
         (c == '\'') ||
         (c == '(') ||
         (c == ')') ||
         (c == '-') ||
         (c == '@') ||
         (c == '^') ||
         (c == '_') ||
         (c == '`') ||
         (c == '{') ||
         (c == '}') ||
         (c == '~');
}


SharedPtr<Vnode> FAT32::get_root_vnode() {
  return _root_inode;
}


FAT32::Entry::Entry() {
  memset(this, 0, 32);
}

FAT32::Entry::Entry(const void* ptr) {
  memcpy(this, ptr, 32);
}

String FAT32::FilenameEntry::get_filename() const {
  size_t len = sizeof(filename_part1) +
               sizeof(filename_part2) +
               sizeof(filename_part3);

  ucs2_char_t filename[14];
  char out[len + 1];

  memcpy(filename, filename_part1, 5 * 2);
  memcpy(filename + 5, filename_part2, 6 * 2);
  memcpy(filename + 11, filename_part3, 2 * 2);
  filename[13] = 0;

  ucs2utf(reinterpret_cast<utf8_char_t*>(out), filename);
  return out;
}


uint32_t FAT32::DirectoryEntry::get_first_cluster_number() const {
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
                       uint32_t parent_cluster_number,
                       uint32_t parent_cluster_offset,
                       off_t size,
                       mode_t mode,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_inode_index++, size, mode, uid, gid),
      _fs(fs),
      _name(name),
      _first_cluster_number(first_cluster_number),
      _parent_cluster_number(parent_cluster_number),
      _parent_cluster_offset(parent_cluster_offset),
      _content() {}


SharedPtr<Vnode> FAT32Inode::create_child(const String& name,
                                          const char* content,
                                          off_t size,
                                          mode_t mode,
                                          uid_t uid,
                                          gid_t gid) {
  List<String> name_segments;
  for (size_t i = 0; i < name.size(); i += 13) {
    name_segments.push_front(name.substr(i, 13));
  }

  // Find `name_segments.size() +` contiguous free entries in the target directory.
  // See if we can find n contiguous free entries in the target directory,
  // where n = name_segment.size() + 1
  //           -------------------   -
  //               # of fentry      # of dentry
  auto cluster = make_unique<char[]>(512);
  uint32_t cluster_number = 0;
  uint32_t cluster_offset = 0;
  uint32_t counter = 0;
  bool keep_searching = true;

  // Follow the directory's cluster chain until EoC is found.
  for (uint32_t n = _first_cluster_number;
       !FAT32_IS_EOC(n) && keep_searching;
       n = _fs.fat_read(n)) {
    _fs.cluster_read(n, cluster.get());

    // Scan each fentry/dentry in this cluster.
    for (char* ptr = cluster.get();
         ptr < cluster.get() + 512 && keep_searching;
         ptr += 32) {
      const FAT32::FilenameEntry entry(ptr);

      if (entry.is_the_end()) {
        cluster_number = n;
        cluster_offset = ptr - cluster.get();
        counter = 0;
        keep_searching = false;
        break;
      }

      if (entry.attributes == ATTR_LFN_ENTRY) {
        if (counter++ == 0) {
          cluster_number = n;
          cluster_offset = ptr - cluster.get();
        }
      } else {
        // The fentries of a deleted dentry are usually untouched
        // even after deletion, so we only confirm whether we can
        // reuse these entries until we've reached the deleted dentry.
        if (!entry.is_deleted()) {
          cluster_number = n;
          cluster_offset = ptr - cluster.get();
          counter = 0;
        } else if (++counter == name_segments.size() + 1) {
          keep_searching = false;
          break;
        }
      }
    }
  }


  // If the counter is 0, then it means there are no reusable
  // contiguous deleted entries.
  //
  // At this point, `cluster_number` and `cluster_offset`
  // will refer to the EoC entry of this directory. We should
  // add n empty entries to this directory.
  if (counter < name_segments.size() + 1) {
    printk("no reusable contiguous deleted entries, appending to the end\n");
    size_t entries_to_allocate = name_segments.size() + 2;  // +2 because for end entry

    // Follow the directory's cluster chain until EoC is found.
    for (uint32_t n = cluster_number;
         !FAT32_IS_EOC(n) && entries_to_allocate > 0;
         n = _fs.fat_read(n)) {
      _fs.cluster_read(n, cluster.get());

      // Scan each fentry/dentry in this cluster.
      for (char* ptr = cluster.get() + cluster_offset;
           ptr < cluster.get() + 512 && entries_to_allocate > 0;
           ptr += 32) {

        if (FAT32::DirectoryEntry(ptr).is_the_end()) {
          *ptr = 0;

          // Allocate a new cluster if needed.
          // Important note: after allocating a new cluster,
          // we also need to make each of its entries start with a null byte.
          if (ptr == cluster.get() + 512 - 32) {
            uint32_t new_cluster_number = _fs.fat_find_free_cluster();
            if (new_cluster_number == static_cast<uint32_t>(-1)) [[unlikely]] {
              return nullptr;
            }
            _fs.fat_write(n, new_cluster_number);
            _fs.fat_write(new_cluster_number, FAT32_EOC_MAX);

            _fs.cluster_read(new_cluster_number, cluster.get());
            for (char* ptr = cluster.get(); ptr < cluster.get() + 512; ptr += 32) {
              *ptr = 0;
            }
            _fs.cluster_write(new_cluster_number, cluster.get());
          }

          entries_to_allocate--;
        }
      }
    }

    counter = name_segments.size() + 1;
  } else {
    printk("found %d reusable slots, cluster = %d, offset = %d\n", counter, cluster_number, cluster_offset);
  }

  /*
  printk("name_segments: [");
  for (const auto& s : name_segments) {
    printf("%s,", s.c_str());
  }
  printf("]\n");
  */

  // Now write the fentry(s) and the dentry to the entries
  // that are referred to by `cluster_number` and `cluster_offset`.
  counter = name_segments.size() + 1;
  bool has_written_0x40 = false;
  auto it = name_segments.begin();

  // Follow the directory's cluster chain until EoC is found.
  for (uint32_t n = cluster_number; !FAT32_IS_EOC(n); n = _fs.fat_read(n), cluster_offset = 0) {
    _fs.cluster_read(n, cluster.get());

    // Scan each fentry/dentry in this cluster.
    for (char* ptr = cluster.get() + cluster_offset; ptr < cluster.get() + 512; ptr += 32) {
      if (--counter > 0) {
        ucs2_char_t name_part_ucs2[14] = {0};
        String name_part_utf8 = *it;
        utf2ucs(name_part_ucs2, reinterpret_cast<const utf8_char_t*>(name_part_utf8.c_str()));

        FAT32::FilenameEntry* fentry = reinterpret_cast<FAT32::FilenameEntry*>(ptr);
        fentry->sequence_number = counter;
        fentry->attributes = ATTR_LFN_ENTRY;
        fentry->first_cluster = 0;
        memcpy(fentry->filename_part1, name_part_ucs2, 5 * 2);
        memcpy(fentry->filename_part2, name_part_ucs2 + 5, 6 * 2);
        memcpy(fentry->filename_part3, name_part_ucs2 + 11, 2 * 2);

        if (!has_written_0x40) {
          fentry->sequence_number |= 0x40;
          has_written_0x40 = true;
        }

        printk("writing fentry: %s (n = %d, offset = %d)\n", name_part_utf8.c_str(), n, ptr - cluster.get());
        _fs.cluster_write(n, cluster.get());
        it++;

      } else {
        // Find an empty entry in the FAT table.
        uint32_t free_cluster_number = _fs.fat_find_free_cluster();

        if (free_cluster_number == static_cast<uint32_t>(-1)) {
          return nullptr;
        }

        auto dentry = reinterpret_cast<FAT32::DirectoryEntry*>(ptr);
        dentry->attributes = 0;
        dentry->first_cluster_high = (free_cluster_number & 0xffff0000) >> 16;
        dentry->first_cluster_low  = (free_cluster_number & 0x0000ffff);
        dentry->size = size;

        memset(dentry, ' ', 11);
        if (name_segments.back().front() < 127) {
          memcpy(dentry->name, name_segments.back().c_str(), 11);
        } else {
          dentry->name[0] = '_';
          dentry->name[1] = '~';
          dentry->name[2] = '1';
        }

        printk("writing dentry (n = %d, offset = %d)\n", n, ptr - cluster.get());
        _fs.cluster_write(n, cluster.get());
        _fs.fat_write(free_cluster_number, FAT32_EOC_MAX);

        printk("---\n");
        return make_shared<FAT32Inode>(_fs,
                                       name,
                                       free_cluster_number,
                                       n,
                                       ptr - cluster.get(),
                                       0,  // dentry->size
                                       mode,
                                       0,
                                       0);
      }
    }
  }

  // Shouldn't have reached here.
  Kernel::panic("should not have reached here\n");
  return nullptr;
}

SharedPtr<Vnode> FAT32Inode::get_child(const String& name) {
  return find_child_if([&name](const auto& v) { return v.name == name; });
}

SharedPtr<Vnode> FAT32Inode::get_ith_child(size_t i) {
  return find_child_if([&i](const auto& v) { return v.index == i; });
}

size_t FAT32Inode::get_children_count() const {
  size_t ret = 0;

  iterate_children([&ret](const auto& v) {
    ret++;
    return false;
  });

  return ret;
}

char* FAT32Inode::get_content() {
  _content = make_unique<char[]>(round_up_to_multiple_of_n(_size, 512));

  int i = 0;
  for (uint32_t n = _first_cluster_number; !FAT32_IS_EOC(n); n = _fs.fat_read(n)) {
    _fs.cluster_read(n, _content.get() + 512 * i++);
  }

  return _content.get();
}

void FAT32Inode::set_content(UniquePtr<char[]> content, off_t new_size) {
  off_t buf_size = round_up_to_multiple_of_n(new_size, 512);

  uint32_t prev_free_cluster_number = 0;
  uint32_t curr_free_cluster_number = _first_cluster_number;

  for (uint32_t i = 0; i < buf_size; i += 512) {
    if (curr_free_cluster_number == static_cast<uint32_t>(-1)) {
      return;
    }

    // Write block to the data region
    _fs.cluster_write(curr_free_cluster_number, content.get() + i);

    // Mark the current cluster as EoC.
    _fs.fat_write(curr_free_cluster_number, FAT32_EOC_MAX);

    // Update File Allocation Table
    if (prev_free_cluster_number) {
      _fs.fat_write(prev_free_cluster_number, curr_free_cluster_number);
    }

    prev_free_cluster_number = curr_free_cluster_number;
    curr_free_cluster_number = _fs.fat_find_free_cluster();
  }


  // Update dentry size in parent's cluster.
  if (FAT32_IS_EOC(_parent_cluster_number)) [[unlikely]] {
    printk("fat32: Are we setting the content of root directory?\n");
    return;
  }

  auto buffer = make_unique<char[]>(_fs._metadata.bytes_per_sector);
  _fs.cluster_read(_parent_cluster_number, buffer.get());
  
  auto dentry
    = reinterpret_cast<FAT32::DirectoryEntry*>(buffer.get() + _parent_cluster_offset);
  dentry->size = new_size;

  _fs.cluster_write(_parent_cluster_number, buffer.get());


  _content = move(content);
  _size = new_size;
}


SharedPtr<FAT32Inode>
FAT32Inode::find_child_if(Function<bool (const FAT32::DirectoryEntryView&)> predicate,
                          uint32_t* out_offset) const {
  FAT32::DirectoryEntryView __dentry_view;
  __dentry_view.index = -1;

  iterate_children([&predicate, &__dentry_view](const auto& v) {
    return predicate(v) &&
           (__dentry_view = v, true);
  });

  if (__dentry_view.index == static_cast<uint32_t>(-1)) {
    return nullptr;
  }


  const FAT32::DirectoryEntryView& dentry_view = __dentry_view;
  mode_t mode = (dentry_view.dentry.attributes & ATTR_DIRECTORY) ? S_IFDIR : S_IFREG;

  return make_shared<FAT32Inode>(_fs,
                                 dentry_view.name,
                                 dentry_view.dentry.get_first_cluster_number(),
                                 dentry_view.parent_cluster_number,
                                 dentry_view.parent_cluster_offset,
                                 dentry_view.dentry.size,
                                 mode,
                                 0,
                                 0);
}

void FAT32Inode::iterate_children(Function<bool (const FAT32::DirectoryEntryView&)> f) const {
  // Make sure this inode represents a directory.
  if (!(_mode & S_IFDIR)) [[unlikely]] {
    printk("fat32: iterate_children(): %s is not a directory\n", _name.c_str());
    return;
  }

  auto cluster = make_unique<char[]>(512);
  FAT32::DirectoryEntryView dentry_view;
  dentry_view.index = 0;

  // Follow the directory's cluster chain until EoC is found.
  for (uint32_t n = _first_cluster_number; !FAT32_IS_EOC(n); n = _fs.fat_read(n)) {
    _fs.cluster_read(n, cluster.get());

    // Scan each fentry/dentry in this cluster.
    for (char* ptr = cluster.get(); ptr < cluster.get() + 512; ptr += 32) {
      const FAT32::FilenameEntry fentry(ptr);

      if (fentry.is_deleted()) {
        /*
        printk("found deleted entry (%d, %d)\n", n, ptr - cluster.get());
        for (int i = 0; i < 13; i++) {
          printf("%x ", ptr[i]);
        }
        printf("\n");
        */
        continue;
      }

      if (fentry.is_the_end()) {
        /*
        printk("found end marker (%d, %d)\n", n, ptr - cluster.get());
        for (int i = 0; i < 13; i++) {
          printf("%x ", ptr[i]);
        }
        printf("\n");
        */
        return;
      }

      if (fentry.attributes == ATTR_LFN_ENTRY) {
        if (fentry.sequence_number & 0x40) {
          dentry_view.name.clear();
        }

        printk("found fentry (%d, %d): %s\n", n, ptr - cluster.get(), fentry.get_filename().c_str());
        dentry_view.name = fentry.get_filename() + dentry_view.name;
      } else {
        printk("found dentry (%d, %d)\n", n, ptr - cluster.get());
        const FAT32::DirectoryEntry dentry(ptr);
        dentry_view.dentry = dentry;
        dentry_view.parent_cluster_number = n;
        dentry_view.parent_cluster_offset = ptr - cluster.get();

        if (f(dentry_view)) {
          return;
        }

        dentry_view.index++;
      }
    }
  }
}

}  // namespace valkyrie::kernel

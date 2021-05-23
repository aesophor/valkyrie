// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_FAT32_H_
#define VALKYRIE_FAT32_H_

#include <Functional.h>
#include <List.h>
#include <Memory.h>
#include <dev/DiskPartition.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>
#include <libs/Encoding.h>

namespace valkyrie::kernel {

// Forward declaration
class FAT32Inode;

class FAT32 final : public FileSystem {
  // Friend declaration
  friend class FAT32Inode;

 public:
  FAT32(DiskPartition& disk_partition);
  virtual ~FAT32() = default;

  virtual SharedPtr<Vnode> get_root_vnode() override;

 private:
  // In FAT32 with LFN (Long File Name) support, there
  // are two types of directory entries:
  //
  // 1. Filename Entry
  // 2. Directory Entry
  //
  // This base class provides the methods that are common to
  // both entries.
  struct [[gnu::packed]] Entry {
    [[gnu::always_inline]] bool is_deleted() const {
      return *reinterpret_cast<const uint8_t*>(this) == 0xe5;
    }

    [[gnu::always_inline]] bool is_end_of_cluster_chain() const {
      return *reinterpret_cast<const uint8_t*>(this) == 0;
    }

   protected:
    Entry();
    Entry(const void* ptr);
  };

  struct [[gnu::packed]] FilenameEntry final : public FAT32::Entry {
    FilenameEntry() = default;
    FilenameEntry(const void* ptr) : FAT32::Entry(ptr) {}
    String get_filename() const;

    uint8_t sequence_number;
    ucs2_char_t filename_part1[5];  // 5 UCS-2 characters
    uint8_t attributes;             // always 0x0f
    uint8_t type;                   // always 0x00 for VFAT LFN
    uint8_t checksum;
    ucs2_char_t filename_part2[6];  // 6 UCS-2 characters
    uint16_t first_cluster;         // always 0x0000
    ucs2_char_t filename_part3[2];  // 2 UCS-2 characters
  };

  struct [[gnu::packed]] DirectoryEntry final : public FAT32::Entry {
    DirectoryEntry() = default;
    DirectoryEntry(const void* ptr) : FAT32::Entry(ptr) {}
    uint32_t get_first_cluster_number() const;

    char name[8];
    char extension[3];
    uint8_t attributes;
    uint8_t __reserved;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t first_cluster_low;
    uint32_t size;
  };

  // A non-owning reference to a FAT32::DirectoryEntry.
  //
  // The existence of this structure is to eliminate the gap
  // between FilenameEntry and DirectoryEntry, where
  // the long filename is stored in FilenameEntry but
  // the rest are stored in DirectoryEntry.
  struct DirectoryEntryView final {
    String name;
    FAT32::DirectoryEntry dentry;
    uint32_t index;  // `index`-th child of its parent
    uint32_t parent_cluster_number;
    uint32_t parent_cluster_offset;
  };

  struct [[gnu::packed]] BootSector final {
    BootSector(DiskPartition& disk_partition);

    uint8_t bootjmp[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t table_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t table_size_16;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t hidden_sector_count;
    uint32_t total_sectors_32;
    // Below fields are extended section (specific to FAT32).
    uint32_t table_size_32;  // # of sectors occupied by one FAT.
    uint16_t extended_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fat_info;
    uint16_t backup_BS_sector;
    uint8_t __reserved_0[12];
    uint8_t drive_number;
    uint8_t __reserved_1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];
  };

  static_assert(sizeof(FilenameEntry) == 32);
  static_assert(sizeof(DirectoryEntry) == 32);
  static_assert(sizeof(BootSector) == 90);


  [[gnu::always_inline]]
  uint32_t fat0_sector_index(const uint32_t entry_index = 0) const {
    return _metadata.reserved_sector_count +
           entry_index / _nr_fat_entries_per_sector;
  }

  [[gnu::always_inline]]
  uint32_t fat1_sector_index(const uint32_t entry_index = 0) const {
    return fat0_sector_index() + _metadata.table_size_32 +
           entry_index / _nr_fat_entries_per_sector;
  }

  [[gnu::always_inline]]
  uint32_t data_region_sector_index() const {
    return fat1_sector_index() + _metadata.table_size_32;
  }

  [[gnu::always_inline]]
  uint32_t cluster_sector_index(const uint32_t i) const {
    return data_region_sector_index() +
           (i - 2) * _metadata.sectors_per_cluster;
  }

  [[gnu::always_inline]]
  uint32_t nr_single_fat_entries() const {
    return _metadata.table_size_32 * _nr_fat_entries_per_sector;
  }


  // Read/Write the i-th entry of the first FAT.
  uint32_t fat_read(const uint32_t i) const;
  void fat_write(const uint32_t i, const uint32_t val) const;
  uint32_t fat_find_free_cluster() const;

  // Read/Write the i-th cluster from/to the data region.
  void cluster_read(const uint32_t i, void* buf) const;
  void cluster_write(const uint32_t i, const void* buf) const;


  DiskPartition& _disk_partition;
  const BootSector _metadata;
  int _nr_fat_entries_per_sector;
  int _next_inode_index;
  SharedPtr<FAT32Inode> _root_inode;
};


class FAT32Inode final : public Vnode {
  // Friend declaration
  friend class FAT32;

 public:
  FAT32Inode(FAT32& fs,
             const String& name,
             uint32_t first_cluster_number,
             uint32_t parent_cluster_number,
             uint32_t parent_cluster_offset,
             off_t size,
             mode_t mode,
             uid_t uid,
             gid_t gid);

  virtual ~FAT32Inode() = default;


  virtual SharedPtr<Vnode> create_child(const String& name,
                                        const char* content,
                                        off_t size,
                                        mode_t mode,
                                        uid_t uid,
                                        gid_t gid) override;
  virtual void add_child(SharedPtr<Vnode> child) override {}
  virtual SharedPtr<Vnode> remove_child(const String& name) override { return nullptr; }
  virtual SharedPtr<Vnode> get_child(const String& name) override;
  virtual SharedPtr<Vnode> get_ith_child(size_t i) override;
  virtual size_t get_children_count() const override;

  virtual int chmod(const mode_t) override { return 0; }
  virtual int chown(const uid_t, const gid_t) override { return 0; }

  virtual const String& get_name() const override { return _name; }
  virtual char* get_content() override;
  virtual void set_content(UniquePtr<char[]> content, off_t new_size) override;

 private:
  SharedPtr<FAT32Inode>
  find_child_if(Function<bool (const FAT32::DirectoryEntryView&)> predicate,
                uint32_t* out_offset = nullptr) const; 

  // This method can be used to:
  // 1. iterate through all children and apply the callback `f` on each children, or
  // 2. iterate through all children and return when the predicate `f` yields true.
  void iterate_children(Function<bool (const FAT32::DirectoryEntryView&)> f) const; 


  FAT32& _fs;
  String _name;

  uint32_t _first_cluster_number;
  uint32_t _parent_cluster_number;
  uint32_t _parent_cluster_offset;
  UniquePtr<char[]> _content;
};


}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FAT32_H_

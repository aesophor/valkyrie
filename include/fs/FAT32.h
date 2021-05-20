// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_FAT32_H_
#define VALKYRIE_FAT32_H_

#include <Functional.h>
#include <List.h>
#include <Memory.h>
#include <dev/DiskPartition.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>

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
  struct [[gnu::packed]] ShortDirectoryEntry final {
    ShortDirectoryEntry();
    ShortDirectoryEntry(void* ptr);

    [[gnu::always_inline]] bool is_deleted() const {
      return reinterpret_cast<const uint8_t&>(name[0]) == 0xe5;
    }

    [[gnu::always_inline]] bool is_end_of_cluster_chain() const {
      return reinterpret_cast<const uint8_t&>(name[0]) == 0;
    }

    [[gnu::always_inline]] operator bool() const {
      return reinterpret_cast<const uint8_t&>(name[0]) != 0;
    }

    String get_filename() const;
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

  static_assert(sizeof(BootSector) == 90);
  static_assert(sizeof(ShortDirectoryEntry) == 32);


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
             uint32_t cluster_number,
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
                                        gid_t gid) override { return nullptr; }
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
  FAT32::ShortDirectoryEntry
  find_child_if(Function<bool (const FAT32::ShortDirectoryEntry&)> predicate); 

  void
  for_each_child(Function<void (const FAT32::ShortDirectoryEntry&)> callback) const; 


  FAT32& _fs;
  String _name;

  uint32_t _cluster_number;
  UniquePtr<char[]> _content;
};


}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FAT32_H_

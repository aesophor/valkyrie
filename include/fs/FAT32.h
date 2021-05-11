// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_FAT32_H_
#define VALKYRIE_FAT32_H_

#include <List.h>
#include <Memory.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>

#define NR_MAX_PARTITIONS  4

namespace valkyrie::kernel {

// Forward declaration
class FAT32;

class FAT32Vnode final : public Vnode {
  // Friend declaration
  friend class FAT32;

 public:
  FAT32Vnode(FAT32& fs,
             FAT32Vnode* parent,
             const String& name,
             mode_t mode,
             uid_t uid,
             gid_t gid);

  virtual ~FAT32Vnode() = default;


  virtual SharedPtr<Vnode> create_child(const String& name,
                                        const char* content,
                                        size_t size,
                                        mode_t mode,
                                        uid_t uid,
                                        gid_t gid) override { return nullptr; }
  virtual void add_child(SharedPtr<Vnode> child) override {}
  virtual SharedPtr<Vnode> remove_child(const String& name) override { return nullptr; }
  virtual SharedPtr<Vnode> get_child(const String& name) override { return nullptr; }
  virtual SharedPtr<Vnode> get_ith_child(size_t i) override { return nullptr; }
  virtual size_t get_children_count() const override { return 0; }

  virtual int chmod(const mode_t mode) override { return 0; }
  virtual int chown(const uid_t uid, const gid_t gid) override { return 0; }

  virtual const String& get_name() const override { return _name; }
  virtual char* get_content() const override { return nullptr; }
  virtual void set_content(UniquePtr<char[]> content) override {}

 private:
  FAT32& _fs;
  String _name;

  FAT32Vnode* _parent;
  List<SharedPtr<FAT32Vnode>> _children;
};


class FAT32 final : public FileSystem {
  // Friend declaration
  friend class FAT32Vnode;

 public:
  FAT32();
  virtual ~FAT32() = default;

  virtual SharedPtr<Vnode> get_root_vnode() override;

 private:
  struct [[gnu::packed]] BootSector final {
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
    uint32_t table_size_32;
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

  struct [[gnu::packed]] PartitionTableEntry final {
    uint8_t drive_attributes;
    uint8_t chs_addr_partition_start[3];
    uint8_t partition_type;
    uint8_t chs_addr_last_parition_sector[3];
    uint32_t lba_addr_partition_start;
    uint32_t nr_sectors;  // in partition
  };

  struct [[gnu::packed]] MasterBootRecord final {
    uint8_t bootstrap[440];
    uint32_t unique_disk_id;
    uint16_t __reserved;
    PartitionTableEntry partitions[NR_MAX_PARTITIONS];
    uint16_t signature;
  };

  struct [[gnu::packed]] DirectoryEntry final {
    char name[8];
    char extension[3];
    uint8_t attributes;
    uint16_t first_cluster_high;
    uint16_t first_clusert_low;
    uint32_t size;
  };

  static_assert(sizeof(BootSector) == 90);
  static_assert(sizeof(PartitionTableEntry) == 16);
  static_assert(sizeof(MasterBootRecord) == 512);


  int _next_vnode_index;
  SharedPtr<FAT32Vnode> _root_vnode;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FAT32_H_

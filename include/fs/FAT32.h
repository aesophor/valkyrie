// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_FAT32_H_
#define VALKYRIE_FAT32_H_

#include <List.h>
#include <Memory.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>

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
  struct [[gnu::packed]] BootSector {
    unsigned char 		bootjmp[3];
    unsigned char 		oem_name[8];
    unsigned short 	        bytes_per_sector;
    unsigned char		sectors_per_cluster;
    unsigned short		reserved_sector_count;
    unsigned char		table_count;
    unsigned short		root_entry_count;
    unsigned short		total_sectors_16;
    unsigned char		media_type;
    unsigned short		table_size_16;
    unsigned short		sectors_per_track;
    unsigned short		head_side_count;
    unsigned int 		hidden_sector_count;
    unsigned int 		total_sectors_32;
    // Below fields are extended section (specific to FAT32).
    unsigned int		table_size_32;
    unsigned short		extended_flags;
    unsigned short		fat_version;
    unsigned int		root_cluster;
    unsigned short		fat_info;
    unsigned short		backup_BS_sector;
    unsigned char 		reserved_0[12];
    unsigned char		drive_number;
    unsigned char 		reserved_1;
    unsigned char		boot_signature;
    unsigned int 		volume_id;
    unsigned char		volume_label[11];
    unsigned char		fat_type_label[8];
  };

  int _next_vnode_index;
  SharedPtr<FAT32Vnode> _root_vnode;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_FAT32_H_

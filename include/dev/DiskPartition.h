// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_DISK_PARTITION_H_
#define VALKYRIE_DISK_PARTITION_H_

#include <Memory.h>
#include <String.h>
#include <fs/FileSystem.h>

namespace valkyrie::kernel {

// Forward Declaration
class StorageDevice;

class DiskPartition final {
 public:
  enum class Type {
    FAT32,
    SIZE
  };

  // Constructor
  DiskPartition(StorageDevice& storage_device,
                uint32_t start_block_index,
                uint32_t end_block_index,
                const String& name); 

  ~DiskPartition() = default;
  DiskPartition(const DiskPartition&) = delete;
  DiskPartition(DiskPartition&&) = delete;
  DiskPartition& operator =(const DiskPartition&) = delete;
  DiskPartition& operator =(DiskPartition&&) = delete;


  void read_block(int block_index, void* buf) const;
  void write_block(int block_index, const void* buf) const;

  DiskPartition::Type detect_partition_type() const;
  uint32_t get_start_block_index() const;
  uint32_t get_end_block_index() const;
  const String& get_name() const;
  SharedPtr<FileSystem> get_filesystem() const;

 private:
  StorageDevice& _storage_device;
  uint32_t _start_block_index;
  uint32_t _end_block_index;
  String _name;
  SharedPtr<FileSystem> _fs;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_DISK_PARTITION_H_

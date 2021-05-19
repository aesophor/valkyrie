// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_STORAGE_DEVICE_H_
#define VALKYRIE_STORAGE_DEVICE_H_

#include <List.h>
#include <Memory.h>
#include <dev/BlockDevice.h>
#include <dev/DiskPartition.h>

namespace valkyrie::kernel {

class StorageDevice : public BlockDevice {
 public:
  StorageDevice(BlockDeviceDriver& driver,
                size_t block_size = PAGE_SIZE);

  virtual ~StorageDevice() override = default;
  StorageDevice(const StorageDevice&) = delete;
  StorageDevice(StorageDevice&&) = delete;
  StorageDevice& operator =(const StorageDevice&) = delete;
  StorageDevice& operator =(StorageDevice&&) = delete;


  virtual void read_block(int block_index, void* buf) override;
  virtual void write_block(int block_index, const void* buf) override;

  DiskPartition& get_first_partition() const;

 protected:
  List<UniquePtr<DiskPartition>> _partitions;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_STORAGE_DEVICE_H_

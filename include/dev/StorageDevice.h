// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_STORAGE_DEVICE_H_
#define VALKYRIE_STORAGE_DEVICE_H_

#include <List.h>
#include <Memory.h>
#include <TypeTraits.h>

#include <dev/BlockDevice.h>
#include <dev/DiskPartition.h>

namespace valkyrie::kernel {

class StorageDevice : public BlockDevice {
  MAKE_NONCOPYABLE(StorageDevice);
  MAKE_NONMOVABLE(StorageDevice);

 public:
  StorageDevice(const String &name, BlockDevice::Driver &driver,
                size_t block_size = PAGE_SIZE);
  virtual ~StorageDevice() override = default;

  virtual void read_block(int block_index, void *buf) override;
  virtual void write_block(int block_index, const void *buf) override;

  DiskPartition &get_root_partition() const;

 protected:
  List<UniquePtr<DiskPartition>> _partitions;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_STORAGE_DEVICE_H_

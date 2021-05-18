// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/DiskPartition.h>

#include <dev/StorageDevice.h>
#include <fs/FAT32.h>

namespace valkyrie::kernel {

DiskPartition::DiskPartition(StorageDevice& storage_device,
                             uint32_t start_block_index,
                             uint32_t end_block_index,
                             const String& name)
    : _storage_device(storage_device),
      _start_block_index(start_block_index),
      _end_block_index(end_block_index),
      _name(name),
      _fs() {

  switch (detect_partition_type()) {
    case Type::FAT32:
      printk("/dev/%s: detected a FAT32 filesystem.\n", _name.c_str());
      _fs = make_unique<FAT32>(*this);
      break;

    default:
      printk("/dev/%s: unable to recognize the filesystem\n", _name.c_str());
      return;
  }
}


void DiskPartition::read_block(int block_index, void* buf) const {
  if (_start_block_index + block_index > _end_block_index) [[unlikely]] {
    printk("/dev/%s: read_block %d out of bound\n", _name.c_str(), block_index);
    return;
  }

  _storage_device.read_block(_start_block_index + block_index, buf);
}

void DiskPartition::write_block(int block_index, const void* buf) const {
  if (_start_block_index + block_index > _end_block_index) [[unlikely]] {
    printk("/dev/%s: write_block %d out of bound\n", _name.c_str(), block_index);
    return;
  }

  _storage_device.write_block(_start_block_index + block_index, buf);
}


DiskPartition::Type DiskPartition::detect_partition_type() const {
  // TODO: 
  return Type::FAT32;
}

uint32_t DiskPartition::get_start_block_index() const {
  return _start_block_index;
}

uint32_t DiskPartition::get_end_block_index() const {
  return _end_block_index;
}

const String& DiskPartition::get_name() const {
  return _name;
}

FileSystem& DiskPartition::get_filesystem() {
  return *_fs;
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_BLOCK_DEVICE_DRIVER_H_
#define VALKYRIE_BLOCK_DEVICE_DRIVER_H_

namespace valkyrie::kernel {

// Block device driver interface
class BlockDeviceDriver {
 public:
  virtual ~BlockDeviceDriver() = default;

  virtual void read_block(int block_index, void* buf) = 0;
  virtual void write_block(int block_index, const void* buf) = 0;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_BLOCK_DEVICE_DRIVER_H_

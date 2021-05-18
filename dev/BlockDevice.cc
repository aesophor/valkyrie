// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/BlockDevice.h>

namespace valkyrie::kernel {

BlockDevice::BlockDevice(BlockDeviceDriver& driver,
                         size_t block_size)
    : _driver(driver),
      _block_size(block_size) {}


size_t BlockDevice::get_block_size() const {
  return _block_size;
}

}  // namespace valkyrie::kernel

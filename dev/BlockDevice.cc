// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/BlockDevice.h>

namespace valkyrie::kernel {

BlockDevice::BlockDevice(const String& name,
                         BlockDevice::Driver& driver,
                         size_t block_size)
    : Device(name),
      _driver(driver),
      _block_size(block_size) {}


bool BlockDevice::is_character_device() const {
  return false;
}

bool BlockDevice::is_block_device() const {
  return true;
}

size_t BlockDevice::get_block_size() const {
  return _block_size;
}

}  // namespace valkyrie::kernel

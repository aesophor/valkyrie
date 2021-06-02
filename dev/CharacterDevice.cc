// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/CharacterDevice.h>

namespace valkyrie::kernel {

CharacterDevice::CharacterDevice(const String& name,
                                 CharacterDevice::Driver& driver)
    : Device(name),
      _driver(driver) {}


bool CharacterDevice::is_character_device() const {
  return false;
}

bool CharacterDevice::is_block_device() const {
  return true;
}

}  // namespace valkyrie::kernel

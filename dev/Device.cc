// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/Device.h>

namespace valkyrie::kernel {

Device::Device(const String& name)
    : _name(name) {}


const String& Device::get_name() const {
  return _name;
}

}  // namespace valkyrie::kernel

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_DEVICE_H_
#define VALKYRIE_DEVICE_H_

namespace valkyrie::kernel {

class Device {
 public:
  Device() = default;

  virtual ~Device() = default;
  Device(const Device&) = delete;
  Device(Device&&) = delete;
  Device& operator =(const Device&) = delete;
  Device& operator =(Device&&) = delete;

  virtual bool is_character_device() const = 0;
  virtual bool is_block_device() const = 0;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_DEVICE_H_

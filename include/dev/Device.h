// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_DEVICE_H_
#define VALKYRIE_DEVICE_H_

#include <Types.h>

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


  // Idea borrowed from Linux kernel. See:
  // https://elixir.bootlin.com/linux/latest/source/include/linux/kdev_t.h
  static constexpr uint32_t get_major(const dev_t dev) {
    return dev >> _minor_bits;
  }

  static constexpr uint32_t get_minor(const dev_t dev) {
    return dev & _minor_mask;
  }

  static constexpr uint32_t encode(const uint32_t major, const uint32_t minor) {
    return (major << _minor_bits) | minor;
  }

 private:
  static constexpr int _minor_bits = 20;
  static constexpr int _minor_mask = (1U << _minor_bits) - 1;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_DEVICE_H_

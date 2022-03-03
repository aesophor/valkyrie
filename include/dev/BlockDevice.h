// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_BLOCK_DEVICE_H_
#define VALKYRIE_BLOCK_DEVICE_H_

#include <Memory.h>
#include <TypeTraits.h>

#include <dev/Device.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

class BlockDevice : public Device {
  MAKE_NONCOPYABLE(BlockDevice);
  MAKE_NONMOVABLE(BlockDevice);

 public:
  class Driver {
   public:
    virtual ~Driver() = default;

    virtual void read_block(int block_index, void *buf) = 0;
    virtual void write_block(int block_index, const void *buf) = 0;
  };

  BlockDevice(const String &name, BlockDevice::Driver &driver, size_t block_size = PAGE_SIZE);
  virtual ~BlockDevice() = default;

  virtual bool is_character_device() const override;
  virtual bool is_block_device() const override;

  virtual void read_block(int block_index, void *buf) = 0;
  virtual void write_block(int block_index, const void *buf) = 0;

  size_t get_block_size() const;

 protected:
  BlockDevice::Driver &_driver;
  size_t _block_size;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_BLOCK_DEVICE_H_

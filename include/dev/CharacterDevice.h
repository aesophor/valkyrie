// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_CHARACTER_DEVICE_H_
#define VALKYRIE_CHARACTER_DEVICE_H_

#include <TypeTraits.h>

#include <dev/Device.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

class CharacterDevice : public Device {
  MAKE_NONCOPYABLE(CharacterDevice);
  MAKE_NONMOVABLE(CharacterDevice);

 public:
  class Driver {
   public:
    virtual ~Driver() = default;

    virtual char read_char() = 0;
    virtual void write_char(const char c) = 0;
  };

  CharacterDevice(const String &name, CharacterDevice::Driver &driver);
  virtual ~CharacterDevice() = default;

  virtual bool is_character_device() const override;
  virtual bool is_block_device() const override;

  virtual char read_char() = 0;
  virtual void write_char(const char c) = 0;

 protected:
  CharacterDevice::Driver &_driver;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_CHARACTER_DEVICE_H_

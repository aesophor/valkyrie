// Copyright (c) 2021 GrassLab @ NYCU. All rights reserved.
#ifndef VALKYRIE_SDCARD_DRIVER
#define VALKYRIE_SDCARD_DRIVER

#include <dev/BlockDeviceDriver.h>
#include <driver/IO.h>
#include <driver/GPIO.h>

namespace valkyrie::kernel {

class SDCardDriver final : public BlockDeviceDriver {
 public:
  static SDCardDriver& get_instance();

  virtual ~SDCardDriver() override = default;
  SDCardDriver(const SDCardDriver&) = delete;
  SDCardDriver(SDCardDriver&&) = delete;
  SDCardDriver& operator =(const SDCardDriver&) = delete;
  SDCardDriver& operator =(SDCardDriver&&) = delete;

  virtual void read_block(int block_idx, void* buf) override;
  virtual void write_block(int block_idx, const void* buf) override;

 private:
  SDCardDriver();

  void pin_setup();  
  void sdhost_setup();
  int sdcard_setup();

  int sd_cmd(uint32_t cmd, uint32_t arg);
  int wait_sd();
  int wait_fifo();
  void wait_finish();
  void set_block(int size, int cnt);


  bool _is_high_capacity;  // SDHC
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SDCARD_DRIVER

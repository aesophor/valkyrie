// Copyright (c) 2021 GrassLab @ NYCU. All rights reserved
#include <dev/SDCardDriver.h>

namespace valkyrie::kernel {

SDCardDriver& SDCardDriver::get_instance() {
  static SDCardDriver instance;
  return instance;
}

SDCardDriver::SDCardDriver()
    : _is_high_capacity() {
  pin_setup();
  sdhost_setup();
  sdcard_setup();
}


void SDCardDriver::pin_setup() {
  io::put<uint32_t>(GPFSEL4, 0x24000000);
  io::put<uint32_t>(GPFSEL5, 0x924);
  io::put<uint32_t>(GPPUD, 0);
  io::delay(15000);
  io::put<uint32_t>(GPPUDCLK1, 0xffffffff);
  io::delay(15000);
  io::put<uint32_t>(GPPUDCLK1, 0);
}

void SDCardDriver::sdhost_setup() {
  uint32_t tmp;

  io::put<uint32_t>(SDHOST_PWR, 0);
  io::put<uint32_t>(SDHOST_CMD, 0);
  io::put<uint32_t>(SDHOST_ARG, 0);
  io::put<uint32_t>(SDHOST_TOUT, SDHOST_TOUT_DEFAULT);
  io::put<uint32_t>(SDHOST_CDIV, 0);
  io::put<uint32_t>(SDHOST_HSTS, SDHOST_HSTS_MASK);
  io::put<uint32_t>(SDHOST_CFG, 0);
  io::put<uint32_t>(SDHOST_CNT, 0);
  io::put<uint32_t>(SDHOST_SIZE, 0);

  tmp = io::get<uint32_t>(SDHOST_DBG);
  tmp &= ~SDHOST_DBG_MASK;
  tmp |= SDHOST_DBG_FIFO;

  io::put<uint32_t>(SDHOST_DBG, tmp);
  io::delay(250000);
  io::put<uint32_t>(SDHOST_PWR, 1);
  io::delay(250000);
  io::put<uint32_t>(SDHOST_CFG, SDHOST_CFG_SLOW | SDHOST_CFG_INTBUS | SDHOST_CFG_DATA_EN);
  io::put<uint32_t>(SDHOST_CDIV, SDHOST_CDIV_DEFAULT);
}

int SDCardDriver::sdcard_setup() {
  uint32_t tmp;

  sd_cmd(GO_IDLE_STATE | SDHOST_NO_REPONSE, 0);
  sd_cmd(SEND_IF_COND, VOLTAGE_CHECK_PATTERN);

  tmp = io::get<uint32_t>(SDHOST_RESP0);
  if (tmp != VOLTAGE_CHECK_PATTERN) {
    return -1;
  }

  while (true) {
    if (sd_cmd(APP_CMD, 0) == -1) {
      // MMC card or invalid card status
      // currently not support
      continue;
    }
    sd_cmd(SD_APP_OP_COND, SDCARD_3_3V | SDCARD_ISHCS);
    tmp = io::get<uint32_t>(SDHOST_RESP0);
    if (tmp & SDCARD_READY) {
      break;
    }
    io::delay(1000000);
  }

  _is_high_capacity = tmp & SDCARD_ISHCS;
  sd_cmd(ALL_SEND_CID | SDHOST_LONG_RESPONSE, 0);
  sd_cmd(SEND_RELATIVE_ADDR, 0);
  tmp = io::get<uint32_t>(SDHOST_RESP0);
  sd_cmd(SELECT_CARD, tmp);
  sd_cmd(SET_BLOCKLEN, 512);
  return 0;
}


void SDCardDriver::read_block(int block_idx, void* buf) {
  uint32_t* buf_u = reinterpret_cast<uint32_t*>(buf);
  bool success = false;

  if (!_is_high_capacity) {
    block_idx <<= 9;
  }

  do {
    set_block(512, 1);
    sd_cmd(READ_SINGLE_BLOCK | SDHOST_READ, block_idx);
    for (int i = 0; i < 128; ++i) {
      wait_fifo();
      buf_u[i] = io::get<uint32_t>(SDHOST_DATA);
    }
    uint32_t hsts;
    hsts = io::get<uint32_t>(SDHOST_HSTS);
    if (hsts & SDHOST_HSTS_ERR_MASK) {
      io::put<uint32_t>(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
      sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
    } else {
      success = true;
    }
  } while (!success);

  wait_finish();
}

void SDCardDriver::write_block(int block_idx, void* buf) {
  uint32_t* buf_u = reinterpret_cast<uint32_t*>(buf);
  bool success = false;

  if (!_is_high_capacity) {
    block_idx <<= 9;
  }

  do {
    set_block(512, 1);
    sd_cmd(WRITE_SINGLE_BLOCK | SDHOST_WRITE, block_idx);
    for (int i = 0; i < 128; ++i) {
      wait_fifo();
      io::put<uint32_t>(SDHOST_DATA, buf_u[i]);
    }
    uint32_t hsts;
    hsts = io::get<uint32_t>(SDHOST_HSTS);
    if (hsts & SDHOST_HSTS_ERR_MASK) {
      io::put<uint32_t>(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
      sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
    } else {
      success = true;
    }
  } while (!success);

  wait_finish();
}


int SDCardDriver::sd_cmd(uint32_t cmd, uint32_t arg) {
  io::put<uint32_t>(SDHOST_ARG, arg);
  io::put<uint32_t>(SDHOST_CMD, cmd | SDHOST_NEW_CMD);
  return wait_sd();
}

int SDCardDriver::wait_sd() {
  int cnt = 1000000;
  uint32_t cmd;

  do {
    if (cnt == 0) {
      return -1;
    }
    cmd = io::get<uint32_t>(SDHOST_CMD);
    --cnt;
  } while (cmd & SDHOST_NEW_CMD);
  return 0;
}

int SDCardDriver::wait_fifo() {
  int cnt = 1000000;
  uint32_t hsts;

  do {
    if (cnt == 0) {
      return -1;
    }
    hsts = io::get<uint32_t>(SDHOST_HSTS);
    --cnt;
  } while ((hsts & SDHOST_HSTS_DATA) == 0);
  return 0;
}

void SDCardDriver::wait_finish() {
  uint32_t dbg;
  do {
    dbg = io::get<uint32_t>(SDHOST_DBG);
  } while ((dbg & SDHOST_DBG_FSM_MASK) != SDHOST_HSTS_DATA);
}

void SDCardDriver::set_block(int size, int cnt) {
  io::put<int>(SDHOST_SIZE, size);
  io::put<int>(SDHOST_CNT, cnt);
}

}  // namespace valkyrie::kernel

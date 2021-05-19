// Copyright (c) 2021 GrassLab @ NYCU. All rights reserved
#include <driver/SDCardDriver.h>

// SD card command
#define GO_IDLE_STATE 0
#define SEND_OP_CMD 1
#define ALL_SEND_CID 2
#define SEND_RELATIVE_ADDR 3
#define SELECT_CARD 7
#define SEND_IF_COND 8
#define VOLTAGE_CHECK_PATTERN 0x1aa
#define STOP_TRANSMISSION 12
#define SET_BLOCKLEN 16
#define READ_SINGLE_BLOCK 17
#define WRITE_SINGLE_BLOCK 24
#define SD_APP_OP_COND 41
#define SDCARD_3_3V (1 << 21)
#define SDCARD_ISHCS (1 << 30)
#define SDCARD_READY (1 << 31)
#define APP_CMD 55

// sdhost
#define SDHOST_BASE (MMIO_BASE + 0x202000)
#define SDHOST_CMD (SDHOST_BASE + 0)
#define SDHOST_READ 0x40
#define SDHOST_WRITE 0x80
#define SDHOST_LONG_RESPONSE 0x200
#define SDHOST_NO_REPONSE 0x400
#define SDHOST_BUSY 0x800
#define SDHOST_NEW_CMD 0x8000
#define SDHOST_ARG (SDHOST_BASE + 0x4) 
#define SDHOST_TOUT (SDHOST_BASE + 0x8)
#define SDHOST_TOUT_DEFAULT 0xf00000
#define SDHOST_CDIV (SDHOST_BASE + 0xc)
#define SDHOST_CDIV_MAXDIV 0x7ff
#define SDHOST_CDIV_DEFAULT 0x148
#define SDHOST_RESP0 (SDHOST_BASE + 0x10)
#define SDHOST_RESP1 (SDHOST_BASE + 0x14)
#define SDHOST_RESP2 (SDHOST_BASE + 0x18)
#define SDHOST_RESP3 (SDHOST_BASE + 0x1c)
#define SDHOST_HSTS (SDHOST_BASE + 0x20)
#define SDHOST_HSTS_MASK (0x7f8)
#define SDHOST_HSTS_ERR_MASK (0xf8)
#define SDHOST_HSTS_DATA (1 << 0)
#define SDHOST_PWR (SDHOST_BASE + 0x30)
#define SDHOST_DBG (SDHOST_BASE + 0x34)
#define SDHOST_DBG_FSM_DATA 1
#define SDHOST_DBG_FSM_MASK 0xf
#define SDHOST_DBG_MASK (0x1f << 14 | 0x1f << 9)
#define SDHOST_DBG_FIFO (0x4 << 14 | 0x4 << 9)
#define SDHOST_CFG (SDHOST_BASE + 0x38)
#define SDHOST_CFG_DATA_EN (1 << 4)
#define SDHOST_CFG_SLOW (1 << 3)
#define SDHOST_CFG_INTBUS (1 << 1)
#define SDHOST_SIZE (SDHOST_BASE + 0x3c)
#define SDHOST_DATA (SDHOST_BASE + 0x40)
#define SDHOST_CNT (SDHOST_BASE + 0x50)

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


void SDCardDriver::read_block(int block_index, void* buf) {
  uint32_t* buf_u = reinterpret_cast<uint32_t*>(buf);
  bool success = false;

  if (!_is_high_capacity) {
    block_index <<= 9;
  }

  do {
    set_block(512, 1);
    sd_cmd(READ_SINGLE_BLOCK | SDHOST_READ, block_index);
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

void SDCardDriver::write_block(int block_index, const void* buf) {
  const uint32_t* buf_u = reinterpret_cast<const uint32_t*>(buf);
  bool success = false;

  if (!_is_high_capacity) {
    block_index <<= 9;
  }

  do {
    set_block(512, 1);
    sd_cmd(WRITE_SINGLE_BLOCK | SDHOST_WRITE, block_index);
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

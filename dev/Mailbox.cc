// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/Mailbox.h>

#define GET_BOARD_MODEL     0x00010001
#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define GET_VC_MEMORY       0x00010006
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

namespace valkyrie::kernel {

Mailbox& Mailbox::get_instance() {
  static Mailbox instance;
  return instance;
}

Mailbox::Mailbox() : _msg() {}


uint32_t Mailbox::get_board_model() {
  _msg.buf_size = 7 * 4;
  _msg.buf_req_resp_code = REQUEST_CODE;
  _msg.tag_identifier = GET_BOARD_MODEL;
  _msg.max_value_buffer_size = 4;
  _msg.tag_req_resp_code = TAG_REQUEST_CODE;
  _msg.value_buf[0] = 0;
  _msg.value_buf[1] = END_TAG;
  call();

  return _msg.value_buf[0];
}

uint32_t Mailbox::get_board_revision() {
  _msg.buf_size = 7 * 4;
  _msg.buf_req_resp_code = REQUEST_CODE;
  _msg.tag_identifier = GET_BOARD_REVISION;
  _msg.max_value_buffer_size = 4;
  _msg.tag_req_resp_code = TAG_REQUEST_CODE;
  _msg.value_buf[0] = 0;
  _msg.value_buf[1] = END_TAG;
  call();

  return _msg.value_buf[0];
}

Pair<uint32_t, uint32_t> Mailbox::get_arm_memory() {
  _msg.buf_size = 8 * 4;
  _msg.buf_req_resp_code = REQUEST_CODE;
  _msg.tag_identifier = GET_ARM_MEMORY;
  _msg.max_value_buffer_size = 8;
  _msg.tag_req_resp_code = TAG_REQUEST_CODE;
  _msg.value_buf[0] = 0;
  _msg.value_buf[1] = 0;
  _msg.value_buf[2] = END_TAG;
  call();

  return {_msg.value_buf[0], _msg.value_buf[1]};
}

Pair<uint32_t, uint32_t> Mailbox::get_vc_memory() {
  _msg.buf_size = 8 * 4;
  _msg.buf_req_resp_code = REQUEST_CODE;
  _msg.tag_identifier = GET_VC_MEMORY;
  _msg.max_value_buffer_size = 8;
  _msg.tag_req_resp_code = TAG_REQUEST_CODE;
  _msg.value_buf[0] = 0;
  _msg.value_buf[1] = 0;
  _msg.value_buf[2] = END_TAG;
  call();

  return {_msg.value_buf[0], _msg.value_buf[1]};
}


void Mailbox::call() {
  // Combine the message address (upper 28 bits)
  // with channel number (lower 4 bits)
  const uint32_t addr = (reinterpret_cast<size_t>(&_msg) & ~0xf) | 0x8;

  // Block until the mailbox is not full.
  while (io::get<uint32_t>(MAILBOX_STATUS) & MAILBOX_FULL);
  io::put<uint32_t>(MAILBOX_WRITE, addr);

  // Block until the mailbox is not empty.
  while (io::get<uint32_t>(MAILBOX_STATUS) & MAILBOX_EMPTY);

  // Block until the response is the one we've asked for.
  while (io::get<uint32_t>(MAILBOX_READ) != addr);
}

}  // namespace valkyrie::kernel

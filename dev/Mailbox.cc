// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Mailbox.h>

#define GET_BOARD_REVISION  0x00010002
#define GET_VC_MEMORY       0x00010006
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

namespace valkyrie::kernel {

Mailbox::Mailbox() : _msg() {}


uint32_t Mailbox::get_board_revision() {
  _msg.buf_size = 7 * 4;
  _msg.buf_req_resp_code = REQUEST_CODE;
  _msg.tag_identifier = GET_BOARD_REVISION;
  _msg.max_value_buffer_size = 4;
  _msg.tag_req_resp_code = TAG_REQUEST_CODE;
  _msg.value_buf[0] = 0;
  _msg.value_buf[1] = 0;
  _msg.tag_end = END_TAG;

  return call<uint32_t>();
}

pair<uint32_t, uint32_t> Mailbox::get_vc_memory() {
  _msg.buf_size = 8 * 4;
  _msg.buf_req_resp_code = REQUEST_CODE;
  _msg.tag_identifier = GET_VC_MEMORY;
  _msg.max_value_buffer_size = 4;
  _msg.tag_req_resp_code = TAG_REQUEST_CODE;
  _msg.value_buf[0] = 0;
  _msg.value_buf[1] = 0;
  _msg.tag_end = END_TAG;

  return call<pair<uint32_t, uint32_t>>();
}

}  // namespace valkyrie::kernel

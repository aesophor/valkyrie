// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MAILBOX_H_
#define VALKYRIE_MAILBOX_H_

#include <Singleton.h>
#include <Types.h>
#include <Utility.h>

#include <driver/IO.h>

#define MAILBOX_BASE (MMIO_BASE + 0xb880)

#define MAILBOX_READ (MAILBOX_BASE)
#define MAILBOX_STATUS (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE (MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY 0x40000000
#define MAILBOX_FULL 0x80000000

#define VALUE_BUF_SIZE 3

namespace valkyrie::kernel {

class Mailbox : public Singleton<Mailbox> {
 public:
  uint32_t get_board_model();
  uint32_t get_board_revision();
  Pair<uint32_t, uint32_t> get_arm_memory();  // base address, size
  Pair<uint32_t, uint32_t> get_vc_memory();   // base address, size

 protected:
  Mailbox();

 private:
  void call();

  // Because only upper 28 bits of message address could be passed,
  // the message array should be correctly aligned.
  // clang-format off
  struct [[gnu::packed, gnu::aligned(0x10)]] Message {
    uint32_t buf_size;               // buffer size (in bytes)
    uint32_t buf_req_resp_code;      // request/response code
    uint32_t tag_identifier;         // tag identifier
    uint32_t max_value_buffer_size;  // maximum of request/response value buffer's size
    uint32_t tag_req_resp_code;
    uint32_t value_buf[VALUE_BUF_SIZE];  // value buffer
  } _msg;
  // clang-format on
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MAILBOX_H_

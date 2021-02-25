// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MAILBOX_H_
#define VALKYRIE_MAILBOX_H_

#include <IO.h>
#include <Types.h>
#include <Utility.h>

#define MAILBOX_BASE   (MMIO_BASE + 0xb880)

#define MAILBOX_READ   (MAILBOX_BASE)
#define MAILBOX_STATUS (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE  (MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY  0x40000000
#define MAILBOX_FULL   0x80000000

#define VALUE_BUF_SIZE 2

namespace valkyrie::kernel {

class Mailbox final {
 public:
  Mailbox();
  ~Mailbox() = default;

  uint32_t get_board_revision();
  pair<uint32_t, uint32_t> get_vc_memory();  // base address, size

 private:
  template <typename T>
  T call();

  // Because only upper 28 bits of message address could be passed,
  // the message array should be correctly aligned.
  [[gnu::packed, gnu::aligned(16)]] struct Message {
    uint32_t buf_size;                   // buffer size (in bytes)
    uint32_t buf_req_resp_code;          // request/response code
    uint32_t tag_identifier;             // tag identifier
    uint32_t max_value_buffer_size;      // maximum of request/response value buffer's size
    uint32_t tag_req_resp_code;                
    uint32_t value_buf[VALUE_BUF_SIZE];  // value buffer
    uint32_t tag_end;
  } _msg;
};


template <typename T>
T Mailbox::call() {
  // Combine the message address (upper 28 bits)
  // with channel number (lower 4 bits)
  const uint32_t addr = reinterpret_cast<size_t>(&_msg) & ~0xf | 0x8;

  // Block until the mailbox is not full.
  while (io::read<uint32_t>(MAILBOX_STATUS) & MAILBOX_FULL);
  io::write(MAILBOX_WRITE, addr);

  // Block until the mailbox is not empty.
  while (io::read<uint32_t>(MAILBOX_STATUS) & MAILBOX_EMPTY);

  // Block until the response is the one we've asked for.
  while (io::read<uint32_t>(MAILBOX_READ) != addr);
  return *reinterpret_cast<T*>(_msg.value_buf);
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MAILBOX_H_

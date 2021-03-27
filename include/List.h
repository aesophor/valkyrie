// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_LIST_H_
#define VALKYRIE_LIST_H_

namespace valkyrie::kernel {

// This is similar to Linux kernel's struct list_head,
// except it is renamed to `struct ListNode`.

struct ListNode final {
  ListNode() : prev(this), next(this) {}
  ~ListNode() = default;

  ListNode* prev;
  ListNode* next;
};


bool is_list_empry(const ListNode* node);

void list_add_head(ListNode* head,
                   ListNode* new_node);

void list_add_tail(ListNode* head,
                   ListNode* new_node);

void list_del(ListNode* node);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_LIST_H_

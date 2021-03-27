// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <List.h>

using valkyrie::kernel::ListNode;

namespace {

bool is_list_add_valid(const ListNode* new_node,
                       const ListNode* prev,
                       const ListNode* next) {
  return next->prev == prev &&
         prev->next == next &&
         new_node != prev &&
         new_node != next;
}

bool is_list_del_entry_valid(const ListNode* entry) {
  return entry->prev->next == entry &&
         entry->next->prev == entry;
}

void list_add(ListNode* new_node,
              ListNode* prev,
              ListNode* next) {
  if (!::is_list_add_valid(new_node, prev, next)) {
    return;
  }
  next->prev = new_node;
  new_node->next = next;
  new_node->prev = prev;
  prev->next = new_node;
}

void list_del(ListNode* prev,
              ListNode* next) {
  next->prev = prev;
  prev->next = next;
}

void list_del_entry(ListNode* entry) {
  if (!::is_list_del_entry_valid(entry)) {
    return;
  }
  ::list_del(entry->prev, entry->next);
}

}  // namespace


namespace valkyrie::kernel {

bool is_list_empty(const ListNode* node) {
  return node->next == node;
}

void list_add_head(ListNode* head,
                   ListNode* new_node) {
  ::list_add(new_node, head, head->next);
}

void list_add_tail(ListNode* head,
                   ListNode* new_node) {
  ::list_add(new_node, head->prev, head);
}

void list_del(ListNode* node) {
  ::list_del_entry(node);
  node->prev = nullptr;
  node->next = nullptr;
}

}  // namespace valkyrie::kernel

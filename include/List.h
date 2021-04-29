// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_LIST_H_
#define VALKYRIE_LIST_H_

#include <Functional.h>
#include <Memory.h>
#include <kernel/Compiler.h>

namespace valkyrie::kernel {

// Linux kernel doubly linked list
template <typename T>
class List {
 public:
  // Constructor
  List()
      : _head(make_unique<Node>()),
        _size() {}

  // Destructor
  ~List() {
    Node* ptr = _head->next;
    while (ptr != _head.get()) {
      Node* next = ptr->next;
      delete ptr;
      ptr = next;
    }
  }

  operator bool() const { return !empty(); }


  template <typename U>
  void push_back(U&& val) {
    Node* new_node = new Node(forward<U>(val));
    list_add(new_node, _head->prev, _head.get());
  }

  template <typename U>
  void push_front(U&& val) {
    Node* new_node = new Node(forward<U>(val));
    list_add(new_node, _head.get(), _head->next);
  }

  void pop_back() {
    list_del_entry(_head->prev);
  }

  void pop_front() {
    list_del_entry(_head->next);
  }

  void erase(int index) {
    Node* node = _head->next;
    while (index--) {
      node = node->next;
    }
    list_del_entry(node);
  }
  
  void remove(const T& val) {
    Node* node = _head->next;
    while (node != _head.get()) {
      if (node->data == val) {
        list_del_entry(node);
        return;
      }
      node = node->next;
    }
  }

  void remove_if(Function<bool (T&)> predicate) {
    for (Node* node = _head->next; node != _head.get(); node = node->next) {
      if (predicate(node->data)) {
        list_del_entry(node);
        return;
      }
    }
  }

  T* find_if(Function<bool (T&)> predicate) {
    for (Node* node = _head->next; node != _head.get(); node = node->next) {
      if (predicate(node->data)) {
        return &(node->data);
      }
    }
    return nullptr;
  }

  void for_each(Function<void (T&)> callback) {
    for (Node* node = _head->next; node != _head.get(); node = node->next) {
      callback(node->data);
    }
  }

  void clear() {
    while (!empty()) {
      pop_front();
    }
  }


  size_t size() const { return _size; }
  bool empty() const { return _head->next == _head.get(); }

  T& front() { return _head->next->data; }
  T& back() { return _head->prev->data; }


 protected:
  struct Node final {
    Node() : prev(this), next(this), data() {}

    template <typename U>
    Node(U&& val) : prev(), next(), data(forward<U>(val)) {}

    Node* prev;
    Node* next;
    T data;
  };

  bool is_list_add_valid(const Node* new_node,
                         const Node* prev,
                         const Node* next) {
    return next->prev == prev &&
           prev->next == next &&
           new_node != prev &&
           new_node != next;
  }

  bool is_list_del_entry_valid(const Node* entry) {
    return entry->prev->next == entry &&
           entry->next->prev == entry;
  }

  void list_add(Node* new_node,
                Node* prev,
                Node* next) {
    if (!is_list_add_valid(new_node, prev, next)) {
      return;
    }
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
    _size++;
  }

  void list_del(Node* prev,
                Node* next) {
    next->prev = prev;
    prev->next = next;
  }

  void list_del_entry(Node* entry) {
    if (!is_list_del_entry_valid(entry)) {
      return;
    }

    list_del(entry->prev, entry->next);
    entry->prev = nullptr;
    entry->next = nullptr;
    delete entry;
    _size--;
  }


  UniquePtr<Node> _head;
  size_t _size;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_LIST_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_LIST_H_
#define VALKYRIE_LIST_H_

#include <Functional.h>
#include <Memory.h>
#include <kernel/Compiler.h>

namespace valkyrie::kernel {

// Forward declaration
template <typename ValueType>
class ListIterator;

// Linux kernel doubly linked list
template <typename T>
class List {
  // Friend declaration
  template <typename ValueType>
  friend class ListIterator;

 public:
  using ValueType = T;
  using ConstIterator = ListIterator<const ValueType>;
  using Iterator = ListIterator<ValueType>;

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

  // Copy constructor
  List(const List& r) {
    *this = r;  // delegate to copy assignment operator
  }

  // Copy assignment operator
  List& operator =(const List& r) {
    _head = make_unique<Node>();
    _size = 0;

    // Deep copy this list.
    r.for_each([this](const auto& val) {
      push_back(val);
    });

    return *this;
  }

  // Move constructor
  List(List&& r) noexcept
      : _head(make_unique<Node>()),
        _size() {
    *this = move(r);  // delegate to move assignment operator
  }

  // Move assignment operator
  List& operator =(List&& r) noexcept {
    _head.swap(r._head);
    _size = r._size;
    r._size = 0;
    return *this;
  }


  operator bool() const { return !empty(); }

  Iterator begin() { return Iterator::begin(*this); }
  Iterator end() { return Iterator::end(*this); }
  ConstIterator begin() const { return ConstIterator::begin(*this); }
  ConstIterator end() const { return ConstIterator::end(*this); }


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

  T* find_if(Function<bool (T&)> predicate) const {
    for (Node* node = _head->next; node != _head.get(); node = node->next) {
      if (predicate(node->data)) {
        return &(node->data);
      }
    }
    return nullptr;
  }

  // FIXME: You might spot something weird here...
  // for_each is marked const, but `callback` takes a non-const lvalue ref
  // as argument. Not sure if this is a bug of g++...
  void for_each(Function<void (T&)> callback) const {
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



template <typename ValueType>
class ListIterator {
  // Friend declaration
  template <typename T>
  friend class List;

 public:
  // Destructor
  ~ListIterator() = default;

  // Copy constructor
  ListIterator(const ListIterator& r)
      : _list(r._list),
        _current(r._current) {}

  // Copy assignment operator
  ListIterator& operator =(const ListIterator& r) {
    _current = r._current;
    return *this;
  }

  bool operator ==(const ListIterator& r) const { return _current == r._current; }
  bool operator !=(const ListIterator& r) const { return _current != r._current; }

  const ValueType& operator*() const { return _current->data; }
  const ValueType* operator->() const { return &(_current->data); }

  ValueType& operator*() { return _current->data; }
  ValueType* operator->() { return &(_current->data); }

  ListIterator operator ++() {
    _current = _current->next;
    return *this;
  }

  ListIterator operator --() {
    _current = _current->prev;
    return *this;
  }

  bool is_end() const {
    return _current == _list._head.get();
  }


 private:
  // Constructor
  ListIterator(List<ValueType>& list,
               typename List<ValueType>::Node* current)
      : _list(list),
        _current(current) {}


  static ListIterator begin(List<ValueType>& list) {
    return {list, list._head->next};
  }

  static ListIterator end(List<ValueType>& list) {
    return {list, list._head.get()};
  }

  List<ValueType>& _list;
  typename List<ValueType>::Node* _current;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_LIST_H_

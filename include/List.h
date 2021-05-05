// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_LIST_H_
#define VALKYRIE_LIST_H_

#include <Functional.h>
#include <Memory.h>
#include <Types.h>
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
    for (const auto& data : r) {
      push_back(data);
    }
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

  Iterator find_if(Function<bool (T&)> predicate) {
    for (Iterator it = begin(); it != end(); it++) {
      if (predicate(*it)) {
        return it;
      }
    }
    return end();
  }

  ConstIterator find_if(Function<bool (const T&)> predicate) const {
    for (ConstIterator it = begin(); it != end(); it++) {
      if (predicate(*it)) {
        return it;
      }
    }
    return end();
  }

  void for_each(Function<void (T&)> callback) {
    for (auto& data : *this) {
      callback(data);
    }
  }

  void for_each(Function<void (const T&)> callback) const {
    for (const auto& data : *this) {
      callback(data);
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
  const T& front() const { return _head->next->data; }
  const T& back() const { return _head->prev->data; }


 protected:
  struct Node final {
    // Default constructor
    Node()
        : prev(this),
          next(this),
          data() {}

    // Universal reference constructor
    template <typename U>
    Node(U&& val)
        : prev(),
          next(),
          data(forward<U>(val)) {}

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
        _current(r._current),
        _index(r._index) {}

  // Copy assignment operator
  ListIterator& operator =(const ListIterator& r) {
    _current = r._current;
    _index = r._index;
    return *this;
  }

  bool operator ==(const ListIterator& r) const { return _current == r._current; }
  bool operator !=(const ListIterator& r) const { return _current != r._current; }

  const ValueType& operator *() const { return _current->data; }
  const ValueType* operator ->() const { return &(_current->data); }

  ValueType& operator *() { return _current->data; }
  ValueType* operator ->() { return &(_current->data); }

  // Prefix increment
  ListIterator& operator ++() {
    _current = _current->next;
    _index++;
    return *this;
  }

  // Prefix decrement
  ListIterator& operator --() {
    _current = _current->prev;
    _index--;
    return *this;
  }

  // Postfix increment
  ListIterator operator ++(int) {
    _current = _current->next;
    _index++;
    return *this;
  }

  // Postfix decrement
  ListIterator operator --(int) {
    _current = _current->prev;
    _index--;
    return *this;
  }

  bool is_end() const {
    return _current == _list._head.get();
  }

  size_t index() const {
    return _index;
  }


  template <typename T>
  typename List<T>::Iterator polymorph() const {
    return typename List<T>::Iterator {_list, _current, _index};
  }


 private:
  // Constructor
  ListIterator(List<ValueType>& list,
               typename List<ValueType>::Node* current,
               size_t index)
      : _list(list),
        _current(current),
        _index() {}


  static ListIterator begin(List<ValueType>& list) {
    return {list, list._head->next, 0};
  }

  static ListIterator end(List<ValueType>& list) {
    return {list, list._head.get(), list._size - 1};
  }

  List<ValueType>& _list;
  typename List<ValueType>::Node* _current;
  size_t _index;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_LIST_H_

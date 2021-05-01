// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ITERATOR_H_
#define VALKYRIE_ITERATOR_H_

#include <Types.h>

namespace valkyrie::kernel {

template <typename Container, typename ValueType>
class ContiguousIterator {
  friend Container;

 public:
  // Destructor
  ~ContiguousIterator() = default;

  // Copy constructor
  ContiguousIterator(const ContiguousIterator& r)
      : _container(r._container),
        _index(r._index) {}

  // Copy assignment operator
  ContiguousIterator& operator =(const ContiguousIterator& r) {
    _index = r._index;
    return *this;
  }

  bool operator ==(const ContiguousIterator& r) const { return _index == r._index; }
  bool operator !=(const ContiguousIterator& r) const { return _index != r._index; }
  bool operator <(const ContiguousIterator& r) const { return _index < r._index; }
  bool operator >(const ContiguousIterator& r) const { return _index > r._index; }
  bool operator <=(const ContiguousIterator& r) const { return _index <= r._index; }
  bool operator >=(const ContiguousIterator& r) const { return _index >= r._index; }

  const ValueType& operator*() const { return _container[_index]; }
  const ValueType* operator->() const { return &_container[_index]; }

  ValueType& operator*() { return _container[_index]; }
  ValueType* operator->() { return &_container[_index]; }

  ContiguousIterator operator ++() {
    ++_index;
    return *this;
  }

  ContiguousIterator operator --() {
    --_index;
    return *this;
  }


  bool is_end() const {
    return _index == ContiguousIterator::end(_container)._index;
  }

  size_t index() const {
    return _index;
  }

 private:
  // Constructor
  ContiguousIterator(Container& container, size_t index)
      : _container(container),
        _index(index) {}

  static ContiguousIterator begin(Container& container) {
    return {container, 0};
  }

  static ContiguousIterator end(Container& container) {
    return {container, container.size()};
  }

  Container& _container;
  size_t _index;
};



template <typename Container, typename ValueType>
class NonContiguousIterator {
  friend Container;

 public:
  // Destructor
  ~NonContiguousIterator() = default;

  // Copy constructor
  NonContiguousIterator(const NonContiguousIterator& r)
      : _current(r._current) {}

  // Copy assignment operator
  NonContiguousIterator& operator =(const NonContiguousIterator& r) {
    _current = r._current;
    return *this;
  }

  bool operator ==(const NonContiguousIterator& r) const { return _current == r._current; }
  bool operator !=(const NonContiguousIterator& r) const { return _current != r._current; }
  bool operator <(const NonContiguousIterator& r) const { return _current < r._current; }
  bool operator >(const NonContiguousIterator& r) const { return _current > r._current; }
  bool operator <=(const NonContiguousIterator& r) const { return _current <= r._current; }
  bool operator >=(const NonContiguousIterator& r) const { return _current >= r._current; }

  const ValueType& operator*() const { return *_current; }
  const ValueType* operator->() const { return _current; }

  ValueType& operator*() { return *_current; }
  ValueType* operator->() { return _current; }

  NonContiguousIterator operator ++() {
    _current = _current->next;
    return *this;
  }

  NonContiguousIterator operator --() {
    _current = _current->prev;
    return *this;
  }


  bool is_end() const {
    return _index == NonContiguousIterator::end(_container)._index;
  }

  size_t index() const {
    return _index;
  }


 private:
  // Constructor
  NonContiguousIterator(Container& container, size_t index)
      : _current(container._head) {}


  static NonContiguousIterator begin(Container& container) {
    return {container, 0};
  }

  static NonContiguousIterator end(Container& container) {
    return {container, container.size()};
  }

  ValueType* _current;  // this can be nullptr, think of `end()`
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ITERATOR_H_

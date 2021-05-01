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

  // Prefix increment
  ContiguousIterator& operator ++() {
    ++_index;
    return *this;
  }

  // Prefix decrement
  ContiguousIterator& operator --() {
    --_index;
    return *this;
  }

  // Postfix increment
  ContiguousIterator operator ++(int) {
    ++_index;
    return *this;
  }

  // Postfix decrement
  ContiguousIterator operator --(int) {
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

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ITERATOR_H_

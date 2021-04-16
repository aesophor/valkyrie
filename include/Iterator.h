// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ITERATOR_H_
#define VALKYRIE_ITERATOR_H_

#include <Types.h>

namespace valkyrie::kernel {

template <typename Container, typename ValueType>
class BasicIterator {
 public:
  friend Container;

  // Destructor
  ~BasicIterator() = default;

  // Copy constructor
  BasicIterator(const BasicIterator& r)
      : _container(r._container),
        _index(r._index) {}

  // Copy assignment operator
  BasicIterator& operator =(const BasicIterator& r) {
    _index = r._index;
    return *this;
  }


  bool operator ==(const BasicIterator& r) const { return _index == r._index; }
  bool operator !=(const BasicIterator& r) const { return _index != r._index; }
  bool operator <(const BasicIterator& r) const { return _index < r._index; }
  bool operator >(const BasicIterator& r) const { return _index > r._index; }
  bool operator <=(const BasicIterator& r) const { return _index <= r._index; }
  bool operator >=(const BasicIterator& r) const { return _index >= r._index; }

  const ValueType& operator*() const { return _container[_index]; }
  const ValueType* operator->() const { return &_container[_index]; }

  ValueType& operator*() { return _container[_index]; }
  ValueType* operator->() { return &_container[_index]; }

  BasicIterator operator ++() {
    ++_index;
    return *this;
  }

  BasicIterator operator --() {
    --_index;
    return *this;
  }


  bool is_end() const {
    return _index == BasicIterator::end(_container)._index;
  }

  size_t index() const {
    return _index;
  }


 private:
  // Constructor
  BasicIterator(Container& container, size_t index)
      : _container(container),
        _index(index) {}


  static BasicIterator begin(Container& container) {
    return { container, 0 };
  }

  static BasicIterator end(Container& container) {
    return { container, container.size() };
  }


  Container& _container;
  size_t _index;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ITERATOR_H_

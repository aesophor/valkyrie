// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ITERATOR_H_
#define VALKYRIE_ITERATOR_H_

#include <Types.h>

namespace valkyrie::kernel {

template <typename Container, typename ValueType>
class Iterator {
 public:
  // Destructor
  ~Iterator() = default;

  // Copy constructor
  Iterator(const Iterator& r)
      : _container(r._container),
        _index(r._index) {}

  // Copy assignment operator
  Iterator& operator =(const Iterator& r) {
    _index = r._index;
    return *this;
  }


  

 private:
  Container& _container;
  size_t _index;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ITERATOR_H_

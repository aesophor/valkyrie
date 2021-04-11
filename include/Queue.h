// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_QUEUE_H_
#define VALKYRIE_QUEUE_H_

#include <Memory.h>
#include <Types.h>

#define DEFAULT_CAPACITY 32

namespace valkyrie::kernel {

template <typename T>
class Queue {
 public:
  explicit
  Queue(int capacity = DEFAULT_CAPACITY)
      : _data(make_unique<T[]>(capacity)),
        _head(),
        _tail(),
        _size(),
        _capacity(capacity) {}

  ~Queue() = default;

  T& operator[] (size_t i) { return _data[i]; }

  void push(T val) {
    _data[_tail] = val;

    if (full()) {
      _head = (_head + 1) % _capacity;
    }
    _tail = (_tail + 1) % _capacity;

    if (_size < _capacity) {
      _size++;
    }
  }

  void pop() {
    _head = (_head + 1) % _capacity;

    if (_size > 0) {
      _size--;
    }
  }

  void clear() { _tail = _head = 0; }

  size_t size() const { return _size; }
  size_t capacity() const { return _capacity; }
  bool empty() const { return _tail == _head; }
  bool full() const { return (_tail + 1) % (int) _capacity == _head; }
  T front() const { return _data[_head]; }
  T back() const { return _data[(_tail - 1 < 0) ? _capacity - 1 : _tail - 1]; }

 protected:
  UniquePtr<T[]> _data;
  int _head;
  int _tail;
  size_t _size;
  const size_t _capacity;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_QUEUE_H_

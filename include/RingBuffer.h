// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_RING_BUFFER_H_
#define VALKYRIE_RING_BUFFER_H_

#include <Iterator.h>
#include <Memory.h>
#include <Types.h>

#define DEFAULT_CAPACITY 32

namespace valkyrie::kernel {

template <typename T>
class RingBuffer {
 public:
  using ValueType = T;
  using ConstIterator = BasicIterator<const RingBuffer, const ValueType>;
  using Iterator = BasicIterator<RingBuffer, ValueType>;

  // Constructor
  explicit
  RingBuffer(int capacity = DEFAULT_CAPACITY)
      : _data(make_unique<T[]>(capacity)),
        _head(),
        _tail(),
        _size(),
        _capacity(capacity) {}

  // Destructor
  ~RingBuffer() = default;

  // Copy constructor
  RingBuffer(const RingBuffer& r) = delete;

  // Copy assignment operator
  RingBuffer& operator =(const RingBuffer& r) = delete;

  // Move constructor
  RingBuffer(RingBuffer&& r) = delete;

  // Move assignment operator
  RingBuffer& operator =(RingBuffer&& r) = delete;


  T& operator[] (size_t i) {
    return _data[i];
  }

  template <typename U>
  void push(U&& val) {
    _data[_tail] = forward<U>(val);

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

  void clear() {
    _tail = _head = 0;
    _size = 0;
  }

  Iterator begin() { return Iterator::begin(*this); }
  Iterator end() { return Iterator::end(*this); }
  ConstIterator begin() const { return ConstIterator::begin(*this); }
  ConstIterator end() const { return ConstIterator::end(*this); }

  size_t size() const { return _size; }
  size_t capacity() const { return _capacity; }
  bool empty() const { return _tail == _head; }
  bool full() const { return (_tail + 1) % static_cast<int>(_capacity) == _head; }

  T& front() { return _data[_head]; }
  T& back() { return _data[(_tail - 1 < 0) ? _capacity - 1 : _tail - 1]; }

 protected:
  UniquePtr<T[]> _data;
  int _head;
  int _tail;
  size_t _size;
  const size_t _capacity;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_RING_BUFFER_H_

// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_DEQUE_H_
#define VALKYRIE_DEQUE_H_

#include <Algorithm.h>
#include <Memory.h>
#include <Types.h>

#define DEFAULT_SIZE (static_cast<size_t>(4))

namespace valkyrie::kernel {

template <typename T>
class Deque {
 public:
  // Constructor
  explicit
  Deque(int init_capacity = DEFAULT_SIZE)
    : _data(make_unique<T[]>(init_capacity)),
      _size(),
      _capacity(init_capacity) {}

  // Destructor
  ~Deque() = default;

  // Copy constructor
  Deque(const Deque& r) {
    *this = r;
  }

  // Copy assignment operator
  Deque& operator =(const Deque& r) {
    // Resize and copy the data over.
    resize(r._capacity);
    for (size_t i = 0; i < r._size; i++) {
      _data[i] = r.at(i);
    }
    _size = r._size;
    _capacity = r._capacity;
    return *this;
  }

  // Move constructor
  Deque(Deque&& r) {
    *this = move(r);
  }

  // Move assignment operator
  Deque& operator =(Deque&& r) {
    _data = move(r._data);
    _size = r._size;
    _capacity = r._capacity;

    r.clear();
    return *this;
  }


  T& operator [](size_t i) {
    return _data[i];
  }

  template <typename U>
  void push_back(U&& val) {
    insert(_size, forward<U>(val));
  }

  template <typename U>
  void push_front(U&& val) {
    insert(0, forward<U>(val));
  }

  void pop_back() {
    erase(_size - 1);
  }

  void pop_front() {
    erase(0);
  }

  // Insert val at the specified index, shifting
  // the remaining elements to the right.
  template <typename U>
  void insert(int index, U&& val) {
    if (_size == _capacity) {
      resize(_capacity * 2);
    }

    for (int i = _size; i > index; i--) {
      _data[i] = forward<U>(_data[i - 1]);
    }
    _data[index] = forward<U>(val);
    _size++;
  }

  void erase(int index) {
    for (int i = index; i < (int) _size - 1; i++) {
      _data[i] = move(_data[i + 1]);
    }
    _size--;

    if (_size <= _capacity / 4) {
      resize(_capacity / 2);
    }
  }

  void remove(const T& val) {
    for (size_t i = 0; i < _size; i++) {
      if (_data[i] == val) {
        erase(i);
        return;
      }
    }
  }

  int find(const T& val) {
    // Linear search
    for (int i = 0; i < _size; i++) {
      if (_data[i] == val) {
        return i;
      }
    }
    return -1;
  }

  void resize(size_t new_capacity, bool migrate_data = true) {
    _capacity = max(DEFAULT_SIZE, new_capacity);
    auto new_data = make_unique<T[]>(_capacity);

    if (migrate_data) {
      for (size_t i = 0; i < _size; i++) {
        new_data[i] = move(_data[i]);
      }
    }

    _data.reset();
    _data = move(new_data);
  }

  void clear() {
    resize(DEFAULT_SIZE, /*migrate_data=*/false);
    _size = 0;
  }

  size_t size() const { return _size; }
  size_t capacity() const { return _capacity; }
  bool empty() const { return _size == 0; }
  const T& at(int index) const { return _data.get()[index]; }

  T& front() { return _data[0]; }
  T& back() { return _data[_size - 1]; }
  const T& front() const { return _data[0]; }
  const T& back() const { return _data[_size - 1]; }

 private:
  UniquePtr<T[]> _data;
  size_t _size;
  size_t _capacity;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_DEQUE_H_

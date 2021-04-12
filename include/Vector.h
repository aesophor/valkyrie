// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_VECTOR_H_
#define VALKYRIE_VECTOR_H_

#include <Algorithm.h>
#include <Memory.h>
#include <Types.h>

#define DEFAULT_SIZE (static_cast<size_t>(4))

namespace valkyrie::kernel {

template <typename T>
class Vector {
 public:
  // Constructor
  Vector(int init_capacity = DEFAULT_SIZE)
    : _data(make_unique<T[]>(init_capacity)),
      _size(),
      _capacity(init_capacity) {}

  // Destructor
  ~Vector() = default;

  // Copy constructor
  Vector(const Vector& other) {
    *this = other;
  }

  // Copy assignment operator
  Vector& operator =(const Vector& other) {
    // Resize and copy the data over.
    resize(other._capacity);
    for (size_t i = 0; i < other._size; i++) {
      _data[i] = other.at(i);
    }
    _size = other._size;
    _capacity = other._capacity;
    return *this;
  }

  // Move constructor
  Vector(Vector&& other) {
    *this = move(other);
  }

  // Move assignment operator
  Vector& operator =(Vector&& other) {
    _data = move(other._data);
    _size = other._size;
    _capacity = other._capacity;

    other.clear();
    return *this;
  }


  T& operator [](size_t i) {
    return _data[i];
  }

  void push_back(T&& val) { insert(_size, forward<T>(val)); }
  void push_front(T&& val) { insert(0, forward<T>(val)); }
  void pop_back() { erase(_size - 1); }
  void pop_front() { erase(0); }

  // Insert val at the specified index, shifting
  // the remaining elements to the right.
  void insert(int index, T&& val) {
    if (_size == _capacity) {
      resize(_capacity * 2);
    }

    for (int i = _size; i > index; i--) {
      _data[i] = forward<T>(_data[i - 1]);
    }
    _data[index] = forward<T>(val);
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

  void resize(size_t new_capacity) {
    new_capacity = max(DEFAULT_SIZE, new_capacity);
    UniquePtr<T[]> new_data = make_unique<T[]>(new_capacity);

    size_t i = 0;
    for (; i < _size; i++) {
      new_data[i] = move(_data[i]);
    }

    _data = move(new_data);
  }

  void clear() {
    _data = make_unique<T[]>(DEFAULT_SIZE);
    _size = 0;
    _capacity = DEFAULT_SIZE;
  }

  size_t size() const { return _size; }
  size_t capacity() const { return _capacity; }
  bool empty() const { return _size == 0; }

  const T& at(int index) const { return _data.get()[index]; }
  const T& front() const { return at(0); }
  const T& back() const { return at(_size - 1); }

 private:
  UniquePtr<T[]> _data;
  size_t _size;
  size_t _capacity;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VECTOR_H_

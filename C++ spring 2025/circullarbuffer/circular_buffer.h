#pragma once

#include <span>
#include <iterator>
#include <iostream>
#include <limits>
#include <stdexcept>

constexpr std::size_t DYNAMIC_CAPACITY = std::numeric_limits<std::size_t>::max();

template <size_t Capacity>
struct StaticCapacity {};

struct DynamicCapacity {
    size_t cap_;
};

template <size_t Capacity>
using CapacityBase = std::conditional_t<Capacity != DYNAMIC_CAPACITY, StaticCapacity<Capacity>, DynamicCapacity>;

/*
=================================================

                 ITERATORS

=================================================
*/

template<typename T, size_t Capacity = DYNAMIC_CAPACITY, bool is_const = false, bool is_reverse = false>
class baseIterator : private CapacityBase<Capacity> {
    static constexpr bool is_static = (Capacity != DYNAMIC_CAPACITY);

public:
  using iterator_category = std::random_access_iterator_tag;
  using size_type = std::size_t;
  using value_type = std::conditional_t<is_const, const T, T>;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using pointer = value_type*;

private:
  size_type capacity() const noexcept {
    if constexpr (Capacity == DYNAMIC_CAPACITY) {
      return this->cap_;
    } else {
      return Capacity;
    }
  }

public:
  baseIterator(T* data, size_type index, size_type head, size_type cap, size_type sz)
        : data_(data),
          index_(index),
          head_(head),
          size_(sz) {

    if constexpr (Capacity == DYNAMIC_CAPACITY) {
      this->cap_ = cap;
    }
  }

  reference operator*() const noexcept {
    if constexpr (!is_reverse) {
        return data_[(head_ + index_) % capacity()];
    } else {
        return data_[(head_ + (size_ - 1 - index_)) % capacity()];
    }
  }

  pointer operator->() const noexcept {
    return &(**this);
  }

/*
-------------------------------------------------

            ARITHMETIC OF ITERATORS

-------------------------------------------------
*/

  baseIterator& operator++() noexcept {
    ++index_;
    return *this;
  }

  baseIterator operator++(int) noexcept {
    baseIterator temp = *this;
    ++(*this);
    return temp;
  }

  baseIterator& operator--() noexcept {
    --index_;
    return *this;
  }

  baseIterator operator--(int) noexcept {
    baseIterator temp = *this;
    --(*this);
    return temp;
  }

  baseIterator& operator+=(size_type n) {
    index_ += n;
    return *this;
  }

  baseIterator operator+(size_type n) const {
    baseIterator temp = *this;
    temp += n;
    return temp;
  }

  baseIterator& operator-=(size_type n) {
    index_ -= n;
    return *this;
  }

  baseIterator operator-(size_type n) const {
    baseIterator temp = *this;
    temp -= n;
    return temp;
  }

/*
-------------------------------------------------

              COMPARISON OF ITERATORS

-------------------------------------------------
*/

  bool operator==(const baseIterator& other) const {
    return index_ == other.index_;
  }

  bool operator!=(const baseIterator& other) const {
    return index_ != other.index_;
  }

  bool operator<(const baseIterator& other) const {
    return index_ < other.index_;
  }

  bool operator<=(const baseIterator& other) const {
    return index_ <= other.index_;
  }

  bool operator>(const baseIterator& other) const {
    return index_ > other.index_;
  }

  bool operator>=(const baseIterator& other) const {
    return index_ >= other.index_;
  }

  difference_type operator-(const baseIterator& other) const {
    return difference_type(index_) - difference_type(other.index_);
  }

  size_type get_index() {
    return index_;
  }

private:
  pointer data_;
  size_type index_;
  size_type head_;
  size_type size_;
};

template<typename T, size_t Capacity, bool is_const, bool is_reverse>
baseIterator<T, Capacity, is_const, is_reverse> operator+(
    typename baseIterator<T, Capacity, is_const, is_reverse>::size_type n,
    const baseIterator<T, Capacity, is_const, is_reverse>& other_iterator) {
  return other_iterator + n;
}

/*
=================================================

               CIRCULAR BUFFER

=================================================
*/

template<typename T, size_t Capacity = DYNAMIC_CAPACITY>
class CircularBuffer : private CapacityBase<Capacity> {
  static constexpr bool is_static = (Capacity != DYNAMIC_CAPACITY);

public:
  using size_type = std::size_t;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;

  using iterator = baseIterator<T, Capacity, false>;
  using const_iterator = baseIterator<T, Capacity, true>;
  using reverse_iterator = baseIterator<T, Capacity, false, true>;
  using const_reverse_iterator = baseIterator<T, Capacity, true, true>;

private:
  pointer data_ = nullptr;
  size_type size_ = 0;
  size_type head_ = 0;
  size_type tail_ = 0;

public:

/*
-------------------------------------------------

                 GETTERS

-------------------------------------------------
*/

  size_type size() const noexcept {
    return size_;
  }

  size_type capacity() const noexcept {
    if constexpr (is_static) {
      return Capacity;
    } else {
      return this->cap_;
    }
  }

  bool empty() const noexcept {
    return size_ == 0;
  }

  bool full() const noexcept {
    return size_ == capacity();
  }

/*
-------------------------------------------------

         CONSTRUCTORS AND DESTRUCTOR

-------------------------------------------------
*/

  explicit CircularBuffer(size_type capacity = Capacity) {
    if constexpr (is_static) {
      if (capacity != Capacity) {
        throw std::invalid_argument("Capacity mismatch");
      }
      data_ = reinterpret_cast<pointer>(::operator new(Capacity * sizeof(T)));;
    } else {
      if (capacity == DYNAMIC_CAPACITY) {
        throw std::invalid_argument("Invalid dynamic capacity");
      }
      this->cap_ = capacity;
      data_ = reinterpret_cast<pointer>(::operator new(this->cap_ * sizeof(T)));
    }
  }

  CircularBuffer(const CircularBuffer& other)
      : size_(other.size_),
        head_(other.head_),
        tail_(other.tail_) {
    if constexpr (!is_static) {
      this->cap_ = other.capacity();
      data_ = reinterpret_cast<pointer>(::operator new(this->cap_ * sizeof(T)));
    } else {
      data_ = reinterpret_cast<pointer>(::operator new(Capacity * sizeof(T)));
    }
    for (size_type i = 0; i < size_; ++i) {
      new (&data_[(head_ + i) % capacity()]) T(other.data_[(other.head_ + i) % other.capacity()]);
    }
  }

  CircularBuffer& operator=(const CircularBuffer& other) {
    if (this != &other) {
      CircularBuffer temp(other);
      swap(temp);
    }
    return *this;
  }

  ~CircularBuffer() {
    if (data_) {
      for (size_type i = 0; i < size_; ++i) {
        data_[(head_ + i) % capacity()].~T();
      }
      ::operator delete(data_);
    }
  }

/*
-------------------------------------------------

               PUSH AND POP

-------------------------------------------------
*/

  void push_front(const_reference value) {
    T temp(value);
    if (full()) {
      pop_back();
    }
    if (head_ == 0) {
      head_ = capacity() - 1;
    } else {
      --head_;
    }
    new (&data_[head_]) T(std::move(temp));
    ++size_;
  }

  void push_back(const_reference value) {
    T temp(value);
    if (size_ == capacity()) {
      pop_front();
    }
    new (&data_[tail_]) T(std::move(temp));
    tail_ = (tail_ + 1) % capacity();
    ++size_;
  }

  void pop_front() {
    if (empty()) {
      throw std::underflow_error("Buffer is empty");
    }
    data_[head_].~T();
    head_ = (head_ + 1) % capacity();
    --size_;
  }

  void pop_back() {
    if (empty()) {
      throw std::underflow_error("Buffer is empty");
    }
    if (tail_ == 0) {
      tail_ = capacity() - 1;
    } else {
      --tail_;
    }
    data_[tail_].~T();
    --size_;
  }

/*
-------------------------------------------------

                   METHODS

-------------------------------------------------
*/

  reference operator[](size_type index) noexcept {
    return data_[(head_ + index) % capacity()];
  }

  const_reference operator[](size_type index) const noexcept {
    return data_[(head_ + index) % capacity()];
  }

  reference at(size_type index) {
    if (index >= size_) {
      throw std::out_of_range("Index out of range");
    }
    return data_[(head_ + index) % capacity()];
  }

  const_reference at(size_type index) const {
    if (index >= size_) {
      throw std::out_of_range("Index out of range");
    }
    return data_[(head_ + index) % capacity()];
  }

  void swap(CircularBuffer& other) noexcept {
    if (this == &other) {
      return;
    }
    std::swap(data_, other.data_);
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
    if constexpr (!is_static) {
        std::swap(this->cap_, other.cap_);
    }
  }

  void insert(baseIterator<T, Capacity, false> pos, const_reference value) {
    difference_type idx = pos - begin();
    if (full()) {
      if (idx == 0) {
        return;
      }
      pop_front();
      --idx;
    }
    for (difference_type i = size_; i > idx; --i) {
      new (data_ + (head_ + i) % capacity()) T(data_[(head_ + i - 1) % capacity()]);
      data_[(head_ + i - 1) % capacity()].~T();
    }
    size_type new_pos = (head_ + idx) % capacity();
    new (data_ + new_pos) T(value);
    ++size_;
  }

  void erase(baseIterator<T, Capacity, false> pos) {
    for (std::size_t i = pos.get_index(); i < size_ - 1; ++i) {
      data_[(head_ + i) % capacity()] = data_[(head_ + i + 1) % capacity()];
    }
    pop_back();
  }

/*
-------------------------------------------------

                CREATE ITERATORS

-------------------------------------------------
*/

  iterator begin() noexcept {
    return iterator(data_, 0, head_, capacity(), size_);
  }

  const_iterator begin() const noexcept {
    return const_iterator(data_, 0, head_, capacity(), size_);
  }

  iterator end() noexcept {
    return iterator(data_, size_, head_, capacity(), size_);
  }

  const_iterator end() const noexcept {
    return const_iterator(data_, size_, head_, capacity(), size_);
  }

  reverse_iterator rbegin() noexcept {
    return reverse_iterator(data_, 0, head_, capacity(), size_);
  }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(data_, 0, head_, capacity(), size_);
  }

  reverse_iterator rend() noexcept {
    return reverse_iterator(data_, size_, head_, capacity(), size_);
  }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(data_, size_, head_, capacity(), size_);
  }

  const_iterator cbegin() const noexcept {
    return const_iterator(data_, 0, head_, capacity(), size_);
  }

  const_iterator cend() const noexcept {
    return const_iterator(data_, size_, head_, capacity(), size_);
  }

  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(data_, 0, head_, capacity(), size_);
  }

  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(data_, size_, head_, capacity(), size_);
  }
};

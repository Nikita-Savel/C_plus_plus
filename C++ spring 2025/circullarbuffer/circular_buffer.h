#pragma once

#include <algorithm>
#include <limits>
#include <new>
#include <span>
#include <stdexcept>

static constexpr std::size_t DYNAMIC_CAPACITY = std::numeric_limits<std::size_t>::max();

/*
=================================================

             STATIC CIRCULAR BUFFER

=================================================
*/

template <typename T, std::size_t Capacity>
class StaticCapacity {
public:
    std::size_t head_ = 0;
    std::size_t size_ = 0;
    alignas(std::max_align_t) char static_data_[sizeof(T) * Capacity];

    StaticCapacity() = default;

    explicit StaticCapacity(std::size_t cap) {
        if (cap != Capacity) throw std::invalid_argument("Capacity mismatch");
    }

    StaticCapacity(const StaticCapacity& other)
        : head_(other.head_),
		size_(other.size_) {
        for (std::size_t i = 0; i < size_; ++i) {
            std::size_t idx = (head_ + i) % Capacity;
            new (data_ptr() + idx) T(other.data_ptr()[(other.head_ + i) % Capacity]);
        }
    }

    StaticCapacity& operator=(const StaticCapacity& other) {
        if (this != &other) {
            for (std::size_t i = 0; i < size_; ++i)
                data_ptr()[(head_ + i) % Capacity].~T();
            head_ = other.head_;
            size_ = other.size_;
            for (std::size_t i = 0; i < size_; ++i) {
                std::size_t idx = (head_ + i) % Capacity;
                new (data_ptr() + idx) T(other.data_ptr()[(other.head_ + i) % Capacity]);
            }
        }
        return *this;
    }

    ~StaticCapacity() noexcept {
        for (std::size_t i = 0; i < size_; ++i) {
            data_ptr()[(head_ + i) % Capacity].~T();
		}
    }

    std::size_t get_head() const noexcept {
       return head_;
    }

    void set_head(std::size_t new_head) noexcept {
       head_ = new_head;
    }

    std::size_t get_size() const noexcept {
       return size_;
    }

    void set_size(std::size_t new_size) noexcept {
       size_ = new_size;
    }

	std::size_t get_capacity() const noexcept {
       return Capacity;
    }

    T* data_ptr() noexcept {
       return reinterpret_cast<T*>(static_data_);
    }

    const T* data_ptr() const noexcept {
       return reinterpret_cast<const T*>(static_data_);
    }
};

/*
=================================================

             DYNAMIC CIRCULAR BUFFER

=================================================
*/

template <typename T>
class DynamicCapacity {
protected:
    std::size_t head_ = 0;
    std::size_t size_ = 0;
    std::size_t capacity_;
    T* data_;

public:
    explicit DynamicCapacity(std::size_t cap)
        : head_(0),
       size_(0),
       capacity_(cap),
       data_(static_cast<T*>(operator new[](cap * sizeof(T))))
    {}

    DynamicCapacity(const DynamicCapacity& other)
        : head_(other.head_),
        size_(other.size_),
        capacity_(other.capacity_),
        data_(static_cast<T*>(operator new[](other.capacity_ * sizeof(T)))) {
        for (std::size_t i = 0; i < size_; ++i) {
            std::size_t idx = (head_ + i) % capacity_;
            new (data_ptr() + idx) T(other.data_ptr()[(other.head_ + i) % capacity_]);
        }
    }

    ~DynamicCapacity() noexcept {
        for (std::size_t i = 0; i < size_; ++i) {
            data_ptr()[(head_ + i) % capacity_].~T();
		}
        operator delete[](data_);
    }

    DynamicCapacity& operator=(const DynamicCapacity& other) {
        if (this != &other) {
            DynamicCapacity tmp(other);
            swap(tmp);
        }
        return *this;
    }

    void swap(DynamicCapacity& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(data_, other.data_);
    }

    std::size_t get_head() const noexcept {
       return head_;
    }

    void set_head(std::size_t new_head) noexcept {
       head_ = new_head;
    }

    std::size_t get_size() const noexcept {
       return size_;
    }

    void set_size(std::size_t new_size) noexcept {
       size_ = new_size;
    }

    std::size_t get_capacity() const noexcept {
       return capacity_;
    }

	T* data_ptr() noexcept {
       return data_;
    }

    const T* data_ptr() const noexcept {
       return data_;
    }
};

/*
=================================================

                 ITERATORS

=================================================
*/
namespace details {

template<typename Derived, typename T, bool IsConst, bool IsDynamic, std::size_t Capacity>
class iterator_common {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;

    static constexpr std::size_t extent = (IsDynamic || Capacity == DYNAMIC_CAPACITY) ? std::dynamic_extent : Capacity;
    using data_span = std::span<std::conditional_t<IsConst, const T, T>, extent>;

    std::size_t pos_ = 0;
    data_span span_;
    std::size_t head_deltaset_ = 0;

    iterator_common() = default;
    iterator_common(data_span span, std::size_t pos, std::size_t head_deltaset) noexcept
        : pos_(pos),
       span_(span),
       head_deltaset_(head_deltaset)
	{}

/*
-------------------------------------------------

            ARITHMETIC OF ITERATORS

-------------------------------------------------
*/

    Derived& operator++() noexcept {
       ++pos_;
       return *static_cast<Derived*>(this);
    }

    Derived operator++(int) noexcept {
		Derived tmp = *static_cast<Derived*>(this);
		++pos_;
		return tmp;
	}

    Derived& operator--() noexcept {
		--pos_;
		return *static_cast<Derived*>(this);
	}

    Derived operator--(int) noexcept {
		Derived tmp = *static_cast<Derived*>(this);
		--pos_;
		return tmp;
	}

    Derived operator+(difference_type n) const noexcept {
		Derived tmp = *static_cast<const Derived*>(this);
		tmp.pos_ += n;
		return tmp;
	}

    Derived operator-(difference_type n) const noexcept {
		Derived tmp = *static_cast<const Derived*>(this);
		tmp.pos_ -= n;
		return tmp;
	}

/*
-------------------------------------------------

              COMPARISON OF ITERATORS

-------------------------------------------------
*/

    bool operator==(const Derived& other) const noexcept {
        return span_.data() == other.span_.data()
            && span_.size() == other.span_.size()
            && pos_ == other.pos_;
    }

    bool operator!=(const Derived& other) const noexcept {
        return !(*this == other);
    }

    bool operator<(const Derived& other) const noexcept {
        return static_cast<const Derived*>(this)->pos_ < other.pos_;
    }

    bool operator<=(const Derived& other) const noexcept {
        return !(other < *static_cast<const Derived*>(this));
    }

    bool operator>(const Derived& other) const noexcept {
        return other < *static_cast<const Derived*>(this);
    }

    bool operator>=(const Derived& other) const noexcept {
        return !(*this < other);
    }

    difference_type operator-(const Derived& other) const noexcept {
        return static_cast<difference_type>(pos_) - static_cast<difference_type>(other.pos_);
    }

    reference operator*() const noexcept {
       return span_[(head_deltaset_ + pos_) % span_.size()];
    }

    pointer operator->() const noexcept {
       return &span_[(head_deltaset_ + pos_) % span_.size()];
    }
};

template<typename Derived, typename T, bool IsConst, bool IsDynamic, std::size_t Capacity>
Derived operator+(typename iterator_common<Derived, T, IsConst, IsDynamic, Capacity>::difference_type n,
                  const iterator_common<Derived, T, IsConst, IsDynamic, Capacity>& other_iterator) noexcept {
    return static_cast<const Derived&>(other_iterator) + n;
}

template<typename T, bool IsConst, std::size_t Capacity>
class baseIterator : public iterator_common<baseIterator<T, IsConst, Capacity>, T, IsConst, (Capacity == DYNAMIC_CAPACITY), Capacity> {
public:
    using base = iterator_common<baseIterator<T, IsConst, Capacity>, T, IsConst, (Capacity == DYNAMIC_CAPACITY), Capacity>;
    using data_span = typename base::data_span;
    baseIterator() = default;
    baseIterator(data_span span, std::size_t pos, std::size_t head_deltaset) noexcept : base(span, pos, head_deltaset) {}

    template<bool OtherIsConst = IsConst, typename = std::enable_if_t<OtherIsConst>>
    baseIterator(const baseIterator<T, !OtherIsConst, Capacity>& other) noexcept
        : base(other.span_, other.pos_, other.head_deltaset_) {}
};
}

/*
=================================================

               CIRCULAR BUFFER

=================================================
*/

template <typename T, std::size_t Capacity = DYNAMIC_CAPACITY>
class CircularBuffer : public std::conditional_t<Capacity == DYNAMIC_CAPACITY,
    DynamicCapacity<T>, StaticCapacity<T, Capacity>> {
    using Storage = std::conditional_t<Capacity == DYNAMIC_CAPACITY,
        DynamicCapacity<T>, StaticCapacity<T, Capacity>>;
public:
    using size_type = std::size_t;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    using iterator = details::baseIterator<T, false, Capacity>;
    using const_iterator = details::baseIterator<T, true, Capacity>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    CircularBuffer() = default;

    explicit CircularBuffer(size_type cap) : Storage(cap) {}

    CircularBuffer(const CircularBuffer&) = default;

    CircularBuffer& operator=(const CircularBuffer&) = default;

    ~CircularBuffer() = default;

private:
    size_type wrap_index(size_type index) const noexcept {
        return index % get_capacity();
    }

    size_type wrap_index(size_type index, size_type offset) const noexcept {
        return (index + offset) % get_capacity();
    }


/*
-------------------------------------------------

               PUSH AND POP

-------------------------------------------------
*/

public:
    void push_front(const_reference value) {
        size_type new_head = wrap_index(get_head() + get_capacity() - 1);
        if (!full()) {
            new (data_ptr() + new_head) T(value);
            set_head(new_head);
            set_size(get_size() + 1);
        } else {
            data_ptr()[new_head] = value;
            set_head(new_head);
        }
    }

    void push_back(const_reference value) {
        size_type idx = wrap_index(get_head() + get_size());
        if (!full()) {
            new (data_ptr() + idx) T(value);
            set_size(get_size() + 1);
        } else {
            size_type h = get_head();
            data_ptr()[h] = value;
            set_head(wrap_index(h + 1));
        }
    }

    void pop_front() {
        if (empty()) {
            throw std::underflow_error("Buffer is empty");
        }
        data_ptr()[get_head()].~T();
        set_head(wrap_index(get_head() + 1));
        set_size(get_size() - 1);
    }

    void pop_back() {
        if (empty()) {
            throw std::underflow_error("Buffer is empty");
        }
        size_type tail = wrap_index(get_head() + get_size() - 1);
        data_ptr()[tail].~T();
        set_size(get_size() - 1);
    }

/*
-------------------------------------------------

                   METHODS

-------------------------------------------------
*/

    size_type capacity() const noexcept {
       return get_capacity();
    }

    size_type size() const noexcept {
       return get_size();
    }

    bool empty() const noexcept {
       return get_size() == 0;
    }

    bool full() const noexcept {
       return get_size() == get_capacity();
    }

    reference operator[](size_type i) noexcept {
        return data_ptr()[wrap_index(get_head() + i)];
    }

    const_reference operator[](size_type i) const noexcept {
        return data_ptr()[wrap_index(get_head() + i)];
    }

    reference at(size_type i) {
        if (i >= get_size()) {
          throw std::out_of_range("Index out of range");
       }
        return (*this)[i];
    }

    const_reference at(size_type i) const {
        if (i >= get_size()) {
          throw std::out_of_range("Index out of range");
       }
        return (*this)[i];
    }

    iterator insert(const_iterator pos, const_reference value) {
        size_type delta = static_cast<size_type>(pos - cbegin());
        if (full()) {
            if (delta == 0) {
             return begin();
          }
            pop_front();
            --delta;
        }
        size_type old = get_size();
        set_size(old + 1);
        for (size_type i = old; i > delta; --i) {
            size_type from = wrap_index(get_head() + i - 1);
            size_type to = wrap_index(get_head() + i);
            new (data_ptr() + to) T(std::move(data_ptr()[from]));
            data_ptr()[from].~T();
        }
        size_type ins = wrap_index(get_head() + delta);
        new (data_ptr() + ins) T(value);
        return iterator(std::span<T, iterator::extent>(data_ptr(), get_capacity()), delta, get_head());
    }

    iterator erase(const_iterator pos) {
        size_type delta = static_cast<size_type>(pos - cbegin());
        size_type idx = wrap_index(get_head() + delta);
        data_ptr()[idx].~T();
        for (size_type i = delta; i < get_size() - 1; ++i) {
            size_type from = wrap_index(get_head() + i + 1);
            size_type to = wrap_index(get_head() + i);
            new (data_ptr() + to) T(std::move(data_ptr()[from]));
            data_ptr()[from].~T();
        }
        set_size(get_size() - 1);
        return iterator(std::span<T, iterator::extent>(data_ptr(), get_capacity()), delta, get_head());
    }

/*
-------------------------------------------------

                CREATE ITERATORS

-------------------------------------------------
*/

    iterator begin() noexcept {
        return iterator(std::span<T, iterator::extent>(data_ptr(), get_capacity()), 0, get_head());
    }

    const_iterator begin() const noexcept {
        return const_iterator(std::span<const T, const_iterator::extent>(data_ptr(), get_capacity()), 0, get_head());
    }

    const_iterator cbegin() const noexcept {
       return begin();
    }

    iterator end() noexcept {
        return iterator(std::span<T, iterator::extent>(data_ptr(), get_capacity()), get_size(), get_head());
    }

    const_iterator end() const noexcept {
        return const_iterator(std::span<const T, const_iterator::extent>(data_ptr(), get_capacity()), get_size(), get_head());
    }

    const_iterator cend() const noexcept {
       return end();
    }

    reverse_iterator rbegin() noexcept {
       return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
       return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
       return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
       return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
       return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
       return const_reverse_iterator(begin());
    }

	using Storage::get_head;
    using Storage::set_head;
    using Storage::get_size;
    using Storage::set_size;
    using Storage::get_capacity;
    using Storage::data_ptr;
};
#pragma once

#include <cstddef>
#include <memory>

/*
=================================================

                  STACK STORAGE

=================================================
*/

template <size_t N>
class StackStorage {
    alignas(std::max_align_t) char data[N]{};
    char* current_pos = data;

public:
    void* allocate(size_t size, size_t alignment) {
        char* ptr = current_pos;
        const size_t space = N - static_cast<size_t>(ptr - data);
        const auto addr = reinterpret_cast<std::uintptr_t>(ptr);
        const size_t adjustment = (alignment - (addr % alignment)) % alignment;
        if (adjustment > space || size > space - adjustment) {
            throw std::bad_alloc();
        }
        char* aligned_ptr = ptr + adjustment;
        current_pos = aligned_ptr + size;
        return aligned_ptr;
    }

    void deallocate(void* ptr, size_t size) noexcept {
        if (static_cast<char*>(ptr) + size == current_pos) {
            current_pos = static_cast<char*>(ptr);
        }
    }

    StackStorage(const StackStorage&) = delete;
    StackStorage& operator=(const StackStorage&) = delete;

    StackStorage() = default;

    [[nodiscard]] std::size_t remaining_space() const {
        return N - static_cast<std::size_t>(current_pos - data);
    }
};

/*
=================================================

                STACK ALLOCATOR

=================================================
*/

template <typename T, size_t N>
class StackAllocator {
    StackStorage<N>* storage;

public:
    using value_type = T;

    explicit StackAllocator(StackStorage<N>& storage)
        : storage(&storage) {}

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other)
        : storage(other.get_storage()) {}

    StackAllocator(const StackAllocator& other) = default;
    StackAllocator& operator=(const StackAllocator& other) = default;

    value_type* allocate(const std::size_t n) {
        return static_cast<value_type*>(
            storage->allocate(n * sizeof(T), alignof(T))
        );
    }

    void deallocate(T* ptr, std::size_t n) noexcept {
        storage->deallocate(ptr, n * sizeof(T));
    }

    StackStorage<N>* get_storage() const {
        return storage;
    }

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    template <typename U>
    StackAllocator<U, N> select_on_container_copy_construction() const noexcept {
        return StackAllocator<U, N>(*storage);
    }

    template <typename U>
    bool operator==(const StackAllocator<U, N>& other) const noexcept {
        return storage == other.get_storage();
    }

    template <typename U>
    bool operator!=(const StackAllocator<U, N>& other) const noexcept {
        return !(*this == other);
    }
};

/*
=================================================

                   ITERATORS

=================================================
*/

namespace detail {

struct BaseNode {
    BaseNode* prev;
    BaseNode* next;

    BaseNode() = default;

    BaseNode(BaseNode* prev, BaseNode* next) : prev(prev), next(next) {}
};

template <typename T>
struct Node : BaseNode {
    T value;

    Node() = default;

    explicit Node(const T& val) : value(val) {}
};

template<typename T, bool is_const = false>
class baseIterator {
    BaseNode* current;
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = std::conditional_t<is_const, const T, T>;
  using difference_type = std::ptrdiff_t;
  using node_type = std::conditional_t<is_const, const BaseNode*, BaseNode*>;
  using node_t = std::conditional_t<is_const, const Node<T>*, Node<T>*>;

  baseIterator() = default;

  explicit baseIterator(BaseNode* node) : current(node) {}

  [[nodiscard]] BaseNode* node() const {
      return current;
  }

  template<bool other_const, typename = std::enable_if_t<is_const >= other_const>>
  baseIterator(const baseIterator<T, other_const>& other)
      : current(other.node()) {}

  value_type& operator*() const {
      return (static_cast<node_t>(current))->value;
  }

  value_type* operator->() const {
    return &(static_cast<node_t>(current))->value;
  }

/*
-------------------------------------------------

            ARITHMETIC OF ITERATORS

-------------------------------------------------
*/

  bool operator==(const baseIterator& other) const noexcept {
    return current == other.current;
  }

  bool operator!=(const baseIterator& other) const noexcept {
    return !(*this == other);
  }

  baseIterator& operator++() noexcept {
      current = current->next;
      return *this;
  }

  baseIterator operator++(int) noexcept {
      baseIterator temp = *this;
      ++(*this);
      return temp;
  }

  baseIterator& operator--() noexcept {
      current = current->prev;
      return *this;
  }

  baseIterator operator--(int) noexcept {
      baseIterator temp = *this;
      --(*this);
      return temp;
  }

  baseIterator& operator+=(std::size_t n) {
    for (size_t i = 0; i < n; ++i) {
      current = current->next;
    }
    return *this;
  }

  baseIterator operator+(std::size_t n) const {
    baseIterator temp = *this;
    temp += n;
    return temp;
  }

  baseIterator& operator-=(std::size_t n) {
      for (size_t i = 0; i < n; ++i) {
        current = current->prev;
      }
      return *this;
  }

  baseIterator operator-(std::size_t n) const {
    baseIterator temp = *this;
    temp -= n;
    return temp;
  }
};
}

/*
=================================================

                    LIST

=================================================
*/

template<typename T, typename Allocator=std::allocator<T>>
class List {

  detail::BaseNode endNode;
  size_t list_size = 0;
  using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<detail::Node<T>>;
  [[no_unique_address]] NodeAlloc alloc;

public:
    using iterator = detail::baseIterator<T>;
    using const_iterator = detail::baseIterator<T, true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using NodeAllocatorType = typename std::allocator_traits<Allocator>::template rebind_alloc<detail::Node<T>>;
    using NodeAllocTraits = std::allocator_traits<NodeAllocatorType>;

/*
-------------------------------------------------

         CONSTRUCTORS AND DESTRUCTOR

-------------------------------------------------
*/

    void create(const size_t count) {
        size_t created = 0;
        try {
            for (; created < count; ++created) {
                detail::Node<T>* newNode = NodeAllocTraits::allocate(alloc, 1);
                try {
                    NodeAllocTraits::construct(alloc, newNode);
                } catch (...) {
                    NodeAllocTraits::deallocate(alloc, newNode, 1);
                    throw;
                }
                detail::BaseNode* last = endNode.prev;
                newNode->prev = last;
                newNode->next = &endNode;
                last->next = newNode;
                endNode.prev = newNode;
                ++list_size;
            }
        } catch (...) {
            while (created--) {
                pop_back();
            }
            throw;
        }
    }

    List() noexcept
        : endNode{&endNode, &endNode},
        alloc(NodeAlloc()) {}

    explicit List(const Allocator& alloc) noexcept
        : endNode{&endNode, &endNode},
        alloc(alloc) {}

    List(const size_t count, const Allocator& allocator = Allocator())
        : endNode{&endNode, &endNode},
        alloc(allocator) {
        create(count);
    }

    List(const size_t count, const T& value, const Allocator& allocator = Allocator())
        : endNode{&endNode, &endNode},
        alloc(allocator) {
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    void swap(List& other) noexcept {
        std::swap(endNode, other.endNode);
        std::swap(list_size, other.list_size);

        if (endNode.next != &endNode) {
            endNode.next->prev = &endNode;
            endNode.prev->next = &endNode;
        }
        if (other.endNode.next != &other.endNode) {
            other.endNode.next->prev = &other.endNode;
            other.endNode.prev->next = &other.endNode;
        }

        if constexpr (NodeAllocTraits::propagate_on_container_swap::value) {
            std::swap(alloc, other.alloc);
        }
    }

    List& operator=(const List& other) {
        if (this != &other) {
            List tmp(other);
            swap(tmp);
            if constexpr (NodeAllocTraits::propagate_on_container_copy_assignment::value) {
                alloc = other.alloc;
            }
        }
        return *this;
    }

    ~List() {
        clear();
    }

/*
-------------------------------------------------

          METHODS, GETTERS AND SETTERS

-------------------------------------------------
*/
    Allocator get_allocator() const {
        return alloc;
    }

    [[nodiscard]] size_t size() const {
        return list_size;
    }

    [[nodiscard]] bool empty() const {
        return list_size == 0;
    }

    void clear() {
        while (!empty()) {
            pop_back();
        }
    }

    iterator insert(const_iterator pos, const T& value) {
        detail::Node<T>* newNode = NodeAllocTraits::allocate(alloc, 1);
        try {
            NodeAllocTraits::construct(alloc, newNode, value);
        } catch (...) {
            NodeAllocTraits::deallocate(alloc, newNode, 1);
            throw;
        }
        detail::BaseNode* currentNode = pos.node();
        detail::BaseNode* prevNode = currentNode->prev;
        detail::BaseNode* nextNode = currentNode;
        newNode->prev = prevNode;
        newNode->next = nextNode;
        prevNode->next = newNode;
        currentNode->prev = newNode;
        ++list_size;
        return iterator(static_cast<detail::BaseNode*>(newNode));
    }

    iterator erase(const_iterator pos) {
        if (empty()) {
            throw std::out_of_range("List is empty");
        }
        detail::BaseNode* nodeToDeleteBase = pos.node();
        detail::Node<T>* nodeToDelete = static_cast<detail::Node<T>*>(nodeToDeleteBase);
        detail::BaseNode* nextNode = nodeToDelete->next;

        nodeToDelete->prev->next = nodeToDelete->next;
        nodeToDelete->next->prev = nodeToDelete->prev;

        NodeAllocTraits::destroy(alloc, nodeToDelete);
        NodeAllocTraits::deallocate(alloc, nodeToDelete, 1);
        --list_size;

        return iterator(nextNode);
    }

/*
-------------------------------------------------

               PUSH AND POP

-------------------------------------------------
*/

    void push_back(const T& value) {
      insert(end(), value);
    }

    void push_front(const T& value) {
      insert(begin(), value);
    }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("List is empty");
        }
        iterator my_iterator = end();
        --my_iterator;
        erase(my_iterator);
    }

    void pop_front() {
        if (empty()) {
            throw std::out_of_range("List is empty");
        }
        erase(begin());
    }

    List(const List& other)
        : endNode{&endNode, &endNode},
        alloc(NodeAllocTraits::select_on_container_copy_construction(other.alloc)) {
        try {
            for (const auto& item : other) {
                push_back(item);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

/*
-------------------------------------------------

                CREATE ITERATORS

-------------------------------------------------
*/

    iterator begin() noexcept {
        return iterator(endNode.next);
    }

    const_iterator begin() const noexcept {
        return const_iterator(const_cast<detail::BaseNode*>(endNode.next));
    }

    iterator end() noexcept {
        return iterator(&endNode);
    }

    const_iterator end() const noexcept {
        return const_iterator(const_cast<detail::BaseNode*>(&endNode));
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(const_cast<detail::BaseNode*>(endNode.next));
    }

    const_iterator cend() const noexcept {
        return const_iterator(const_cast<detail::BaseNode*>(&endNode));
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }
};

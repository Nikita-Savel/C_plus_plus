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
    alignas(alignof(std::max_align_t)) char data[N]{};
    char* current_pos = data;

public:
    char* get_data() {
      return data;
    }

    [[nodiscard]] char* get_current_pos() const {
        return current_pos;
    }

    void set_current_pos(char* new_pos) {
        current_pos = new_pos;
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
        const std::size_t total_size = sizeof(T) * n;
        std::size_t space = storage->remaining_space();
        void* ptr = storage->get_current_pos();
        void* aligned_ptr = std::align(alignof(T), total_size, ptr, space);
        if (!aligned_ptr) {
            throw std::bad_alloc();
        }
        storage->set_current_pos(static_cast<char*>(aligned_ptr) + total_size);
        return static_cast<value_type*>(aligned_ptr);
    }

    void deallocate(T*, std::size_t) noexcept {}

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

struct BaseNode {
    BaseNode* prev;
    BaseNode* next;

    BaseNode() : prev(nullptr), next(nullptr) {}

    BaseNode(BaseNode* prev, BaseNode* next) : prev(prev), next(next) {}
};

template <typename T>
struct Node : BaseNode {
    T value;

    Node() : value() {}

    explicit Node(const T& val) : value(val) {}
};

template<typename T, bool is_const = false, bool is_reverse = false>
class baseIterator {
    BaseNode* current;
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = std::conditional_t<is_const, const T, T>;
  using difference_type = std::ptrdiff_t;
  using node_type = std::conditional_t<is_const, const BaseNode*, BaseNode*>;
  using node_t = std::conditional_t<is_const, const Node<T>*, Node<T>*>;

  baseIterator() : current(nullptr) {}

  explicit baseIterator(BaseNode* node) : current(node) {}

  [[nodiscard]] BaseNode* node() const {
      return current;
  }

  // baseIterator(const baseIterator<T, false, is_reverse>& other)
  //     : current(other.node()) {}

  template<bool other_const, typename = std::enable_if_t<is_const >= other_const>>
  baseIterator(const baseIterator<T, other_const, is_reverse>& other)
      : current(other.node()) {}

  value_type& operator*() const {
      return (static_cast<node_t>(current))->value;
  }

  value_type* operator->() const {
    return &(static_cast<node_t>(current))->value;
  }

  baseIterator<T, is_const> base() const noexcept {
    if constexpr (is_reverse) {
        return baseIterator<T, is_const>(current->next);
    }
    else {
        return baseIterator<T, is_const>(current);
    }
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
      if constexpr (is_reverse) {
          current = current->prev;
      } else {
          current = current->next;
      }
      return *this;
  }

  baseIterator operator++(int) noexcept {
    baseIterator temp = *this;
    ++(*this);
    return temp;
  }

  baseIterator& operator--() noexcept {
      if constexpr (is_reverse) {
          current = current->next;
      } else {
          current = current->prev;
      }
      return *this;
  }

  baseIterator operator--(int) noexcept {
    baseIterator temp = *this;
    --(*this);
    return temp;
  }

  baseIterator& operator+=(std::size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (is_reverse) {
            current = current->prev;
        } else {
            current = current->next;
        }
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
          if (is_reverse) {
              current = current->next;
          } else {
              current = current->prev;
          }
      }
      return *this;
  }

  baseIterator operator-(std::size_t n) const {
    baseIterator temp = *this;
    temp -= n;
    return temp;
  }
};

/*
=================================================

                    LIST

=================================================
*/

template<typename T, typename Allocator=std::allocator<T>>
class List {

  BaseNode endNode;
  size_t list_size = 0;
  using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node<T>>;
  NodeAlloc alloc;

public:
    using iterator = baseIterator<T>;
    using const_iterator = baseIterator<T, true>;
    using reverse_iterator = baseIterator<T, false, true>;
    using const_reverse_iterator = baseIterator<T, true, true>;

    using NodeAllocatorType = typename std::allocator_traits<Allocator>::template rebind_alloc<Node<T>>;
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
                Node<T>* newNode = NodeAllocTraits::allocate(alloc, 1);
                try {
                    NodeAllocTraits::construct(alloc, newNode);
                } catch (...) {
                    NodeAllocTraits::deallocate(alloc, newNode, 1);
                    throw;
                }
                BaseNode* last = endNode.prev;
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

    List(const size_t count)
        : endNode{&endNode, &endNode},
        alloc(NodeAlloc()) {
            create(count);
    }

    List(const size_t count, const T& value)
        : endNode{&endNode, &endNode},
        alloc(NodeAlloc()) {
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }
    
    explicit List(const Allocator& alloc) noexcept
        : endNode{&endNode, &endNode},
        alloc(alloc) {}

    List(const size_t count, const Allocator& allocator)
        : endNode{&endNode, &endNode},
        alloc(allocator) {
        create(count);
    }

    List(const size_t count, const T& value, const Allocator& allocator)
        : endNode{&endNode, &endNode},
        alloc(allocator) {
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    List(const List& other)
        : endNode{&endNode, &endNode},
        alloc(NodeAllocTraits::select_on_container_copy_construction(other.alloc)) {
            try {
                for (auto it = other.begin(); it != other.end(); ++it) {
                    Node<T>* newNode = NodeAllocTraits::allocate(alloc, 1);
                    try {
                        NodeAllocTraits::construct(alloc, newNode, *it);
                    } catch (...) {
                        NodeAllocTraits::deallocate(alloc, newNode, 1);
                        throw;
                    }
                    newNode->prev = endNode.prev;
                    newNode->next = &endNode;
                    endNode.prev->next = newNode;
                    endNode.prev = newNode;
                    ++list_size;
                }
            } catch (...) {
                clear();
                throw;
            }
    }

    List& operator=(const List& other) {
        if (this != &other) {
            List tmp(other);
            std::swap(endNode, tmp.endNode);
            std::swap(list_size, tmp.list_size);
            if (endNode.next != &endNode) {
                endNode.next->prev = &endNode;
                endNode.prev->next = &endNode;
            }
            if (tmp.endNode.next != &tmp.endNode) {
                tmp.endNode.next->prev = &tmp.endNode;
                tmp.endNode.prev->next = &tmp.endNode;
            }
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
        Node<T>* newNode = NodeAllocTraits::allocate(alloc, 1);
        try {
            NodeAllocTraits::construct(alloc, newNode, value);
        } catch (...) {
            NodeAllocTraits::deallocate(alloc, newNode, 1);
            throw;
        }
        BaseNode* currentNode = pos.node();
        BaseNode* prevNode = currentNode->prev;
        BaseNode* nextNode = currentNode;
        newNode->prev = prevNode;
        newNode->next = nextNode;
        prevNode->next = newNode;
        currentNode->prev = newNode;
        ++list_size;
        return iterator(static_cast<BaseNode*>(newNode));
    }

    iterator erase(const_iterator pos) {
        if (empty()) {
            throw std::out_of_range("List is empty");
        }
        BaseNode* nodeToDeleteBase = pos.node();
        Node<T>* nodeToDelete = static_cast<Node<T>*>(nodeToDeleteBase);
        BaseNode* nextNode = nodeToDelete->next;

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

/*
-------------------------------------------------

                CREATE ITERATORS

-------------------------------------------------
*/

    iterator begin() noexcept {
        return iterator(endNode.next);
    }

    const_iterator begin() const noexcept {
        return const_iterator(const_cast<BaseNode*>(endNode.next));
    }

    iterator end() noexcept {
        return iterator(&endNode);
    }

    const_iterator end() const noexcept {
        return const_iterator(const_cast<BaseNode*>(&endNode));
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(endNode.prev);
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(const_cast<BaseNode*>(endNode.prev));
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(&endNode);
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(const_cast<BaseNode*>(&endNode));
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(const_cast<BaseNode*>(endNode.next));
    }

    const_iterator cend() const noexcept {
        return const_iterator(const_cast<BaseNode*>(&endNode));
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(const_cast<BaseNode*>(endNode.prev));
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(const_cast<BaseNode*>(&endNode));
    }
};

#pragma once

#include <vector>
#include <memory>
#include <stdexcept>

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
    explicit StackAllocator(const StackAllocator<U, N>& other)
        : storage(other.storage) {}


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

    void deallocate(T*, size_t) noexcept {}

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

template <typename T>
struct Node {
    T value;
    Node* prev = nullptr;
    Node* next = nullptr;

    template <typename... Args>
    Node(Args&&... args) : value(std::forward<Args>(args)...) {}
};

/*
=================================================

                     LIST

=================================================
*/

template <typename T, typename Allocator = std::allocator<T>>
class List {
private:
    using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node<T>>;
    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

    Node<T>* head_ = nullptr;
    Node<T>* tail_ = nullptr;
    NodeAlloc alloc;
    size_t size_ = 0;

public:

/*
-------------------------------------------------

         CONSTRUCTORS AND DESTRUCTOR

-------------------------------------------------
*/

    explicit List(const Allocator& alloc_ = Allocator()) : alloc(alloc_) {
        head_ = NodeAllocTraits::allocate(alloc, 1);
        tail_ = NodeAllocTraits::allocate(alloc, 1);
        head_->next = tail_;
        tail_->prev = head_;
    }

    List(const List& other) : List(NodeAllocTraits::select_on_container_copy_construction(other.alloc)) {
        for (const auto& val : other) {
            push_back(val);
        }
    }

    List(List&& other) noexcept
        : head_(other.head_),
        tail_(other.tail_),
        alloc(std::move(other.alloc)),
        size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }

        if (NodeAllocTraits::propagate_on_container_copy_assignment::value) {
            if (alloc != other.alloc) {
                clear();
                alloc = other.alloc;
            }
        }
        clear();
        for (const auto& val : other) {
            push_back(val);
        }
        return *this;
    }

    List& operator=(List&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (head_ && tail_) {
            clear();
            NodeAllocTraits::deallocate(alloc, head_, 1);
            NodeAllocTraits::deallocate(alloc, tail_, 1);
        }

        if (NodeAllocTraits::propagate_on_container_move_assignment::value) {
            alloc = std::move(other.alloc);
        }

        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;

        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
        return *this;
    }

    ~List() {
        if (head_ && tail_) {
            clear();
            NodeAllocTraits::deallocate(alloc, head_, 1);
            NodeAllocTraits::deallocate(alloc, tail_, 1);
        }
    }

/*
-------------------------------------------------

          METHODS, GETTERS AND SETTERS

-------------------------------------------------
*/

    const Allocator& get_allocator() const noexcept {
        return alloc;
    }

    [[nodiscard]] size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    void clear() {
        if (!head_ || !tail_) {
            size_ = 0;
            return;
        }

        Node<T>* current = head_->next;
        while (current != tail_) {
            Node<T>* next_node = current->next;
            NodeAllocTraits::destroy(alloc, std::addressof(current->value));
            NodeAllocTraits::deallocate(alloc, current, 1);
            current = next_node;
        }
        head_->next = tail_;
        tail_->prev = head_;
        size_ = 0;
    }

    void swap(List& other) {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
        if (NodeAllocTraits::propagate_on_container_swap::value) {
            std::swap(alloc, other.alloc);
        }
    }

/*
-------------------------------------------------

              CREATE LIST ITERATORS

-------------------------------------------------
*/

    template <bool IsConst>
    class ListIterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using iterator_category = std::bidirectional_iterator_tag;

        Node<T>* node_ptr_ = nullptr;

        ListIterator() = default;
        explicit ListIterator(Node<T>* node_ptr) : node_ptr_(node_ptr) {}

        operator ListIterator<true>() const {
            return ListIterator<true>(node_ptr_);
        }

        reference operator*() const {
            return node_ptr_->value;
        }

        pointer operator->() const {
            return &(node_ptr_->value);
        }

        ListIterator& operator++() {
            node_ptr_ = node_ptr_->next;
            return *this;
        }

        ListIterator operator++(int) {
            ListIterator temp = *this;
            ++(*this);
            return temp;
        }

        ListIterator& operator--() {
            node_ptr_ = node_ptr_->prev;
            return *this;
        }

        ListIterator operator--(int) {
            ListIterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const ListIterator& other) const {
            return node_ptr_ == other.node_ptr_;
        }

        bool operator!=(const ListIterator& other) const {
            return node_ptr_ != other.node_ptr_;
        }

        Node<T>* node() const {
            return node_ptr_;
        }
    };

    using iterator = ListIterator<false>;
    using const_iterator = ListIterator<true>;

    iterator begin() noexcept {
        return iterator(head_->next);
    }

    const_iterator begin() const noexcept {
        return const_iterator(head_->next);
    }

    iterator end() noexcept {
        return iterator(tail_);
    }

    const_iterator end() const noexcept {
        return const_iterator(tail_);
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator cend() const noexcept {
        return end();
    }

/*
-------------------------------------------------

               PUSH AND POP

-------------------------------------------------
*/

    void push_back(const T& value) {
        emplace(end(), value);
    }

    void push_back(T&& value) {
        emplace(end(), std::move(value));
    }

    void push_front(const T& value) {
        emplace(begin(), value);
    }

    void push_front(T&& value) {
        emplace(begin(), std::move(value));
    }

    void pop_back() {
        erase(std::prev(end()));
    }

    void pop_front() {
        erase(begin());
    }

/*
-------------------------------------------------

                 EMPLACE

-------------------------------------------------
*/

    template<typename... UArgs>
    iterator emplace(const_iterator pos, UArgs&&... args) {
        Node<T>* newNode = NodeAllocTraits::allocate(alloc, 1);
        auto&& first_arg = std::get<0>(std::forward_as_tuple(args...));

        if constexpr (std::is_same_v<std::decay_t<decltype(first_arg)>, T> &&
            std::is_same_v<T, std::pair<typename T::first_type, typename T::second_type>>) {
            if constexpr (std::is_rvalue_reference_v<UArgs&&...>) {
                auto&& key = std::move(const_cast<std::remove_const_t<typename T::first_type>&>(first_arg.first));
                auto&& value = std::move(first_arg.second);

                NodeAllocTraits::construct(alloc, std::addressof(newNode->value),
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(std::move(value)));
            } else {
                NodeAllocTraits::construct(alloc, std::addressof(newNode->value),
                    std::piecewise_construct,
                    std::forward_as_tuple(first_arg.first),
                    std::forward_as_tuple(first_arg.second));
            }
        } else {
            NodeAllocTraits::construct(alloc, std::addressof(newNode->value), std::forward<UArgs>(args)...);
        }

        Node<T>* next_node = pos.node_ptr_;
        Node<T>* prev_node = next_node->prev;

        newNode->next = next_node;
        newNode->prev = prev_node;
        prev_node->next = newNode;
        next_node->prev = newNode;
        size_++;
        return iterator(newNode);
    }

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    iterator erase(const_iterator pos) {
        if (pos == end()) {
            return end();
        }
        Node<T>* node_to_erase = pos.node_ptr_;
        Node<T>* prev_node = node_to_erase->prev;
        Node<T>* next_node = node_to_erase->next;

        prev_node->next = next_node;
        next_node->prev = prev_node;

        NodeAllocTraits::destroy(alloc, std::addressof(node_to_erase->value));
        NodeAllocTraits::deallocate(alloc, node_to_erase, 1);
        size_--;
        return iterator(next_node);
    }

    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return iterator(last.node_ptr_);
    }

    iterator move_node(const_iterator from, const_iterator to) {
        Node<T>* node_to_move = from.node_ptr_;
        Node<T>* target_pos = to.node_ptr_;

        node_to_move->prev->next = node_to_move->next;
        node_to_move->next->prev = node_to_move->prev;

        Node<T>* prev_node = target_pos->prev;
        prev_node->next = node_to_move;
        target_pos->prev = node_to_move;
        node_to_move->prev = prev_node;
        node_to_move->next = target_pos;

        return iterator(node_to_move);
    }
};

/*
=================================================

                UNORDERED MAP

=================================================
*/

template <typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename EqualTo = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    static constexpr size_t INIT_BUCKET_SIZE = 16;
    using NodeType = std::pair<const Key, Value>;
    using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType>;
    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

private:
    struct Bucket {
        typename List<NodeType, NodeAlloc>::iterator begin;
        typename List<NodeType, NodeAlloc>::iterator end;

        Bucket() = default;
        Bucket(const Bucket&) = default;
        Bucket(Bucket&&) = default;
        Bucket& operator=(const Bucket&) = default;
        Bucket& operator=(Bucket&&) = default;
    };

    using BucketAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Bucket>;

    List<NodeType, NodeAlloc> elements;
    std::vector<Bucket, BucketAlloc> buckets;
    [[no_unique_address]] Hash hasher;
    [[no_unique_address]] EqualTo key_equal;
    [[no_unique_address]] Alloc alloc;

    size_t get_bucket_index(const Key& key) const {
        if (buckets.empty()) {
            return 0;
        }
        size_t hash_value = hasher(key);
        return hash_value % buckets.size();
    }

    void check_rehash() {
        if (load_factor() > max_load_factor()) {
            rehash(buckets.size() * 2);
        }
    }

public:
    template <bool IsConst>
    class MapIterator {
        using BaseIterator = std::conditional_t<IsConst,
            typename List<NodeType, NodeAlloc>::const_iterator,
            typename List<NodeType, NodeAlloc>::iterator>;
        BaseIterator it_{};

    public:
        using value_type = typename BaseIterator::value_type;
        using difference_type = typename BaseIterator::difference_type;
        using pointer = typename BaseIterator::pointer;
        using reference = typename BaseIterator::reference;
        using iterator_category = typename BaseIterator::iterator_category;

        MapIterator() = default;
        explicit MapIterator(BaseIterator it) : it_(it) {}

        template <bool B = IsConst, typename = std::enable_if_t<B>>
        MapIterator(const MapIterator<false>& other) : it_(other.get_iterator()) {}

        BaseIterator get_iterator() const {
            return it_;
        }

        reference operator*() const {
            return *it_;
        }

        pointer operator->() const {
            return it_.operator->();
        }

        MapIterator& operator++() {
            ++it_;
            return *this;
        }

        MapIterator operator++(int) {
            MapIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        MapIterator& operator--() {
            --it_;
            return *this;
        }

        MapIterator operator--(int) {
            MapIterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const MapIterator& other) const {
            return it_ == other.it_;
        }

        bool operator!=(const MapIterator& other) const {
            return it_ != other.it_;
        }
    };

    using iterator = MapIterator<false>;
    using const_iterator = MapIterator<true>;

/*
-------------------------------------------------

         CONSTRUCTORS AND DESTRUCTOR

-------------------------------------------------
*/

    UnorderedMap() : UnorderedMap(INIT_BUCKET_SIZE, Hash(), EqualTo(), Alloc()) {}

    explicit UnorderedMap(size_t bucket_count, const Hash& hash = Hash(), const EqualTo& equal = EqualTo(), const Alloc& alloc_ = Alloc())
        : elements(NodeAlloc(alloc_)),
        buckets(bucket_count, BucketAlloc(alloc_)),
        hasher(hash),
        key_equal(equal),
        alloc(alloc_) {
        for (size_t i = 0; i < buckets.size(); ++i) {
            buckets[i].begin = elements.end();
            buckets[i].end = elements.end();
        }
    }

    UnorderedMap(const UnorderedMap& other)
        : elements(NodeAllocTraits::select_on_container_copy_construction(other.alloc)),
        buckets(other.buckets.size(), BucketAlloc(NodeAllocTraits::select_on_container_copy_construction(other.alloc))),
        hasher(other.hasher),
        key_equal(other.key_equal),
        alloc(NodeAllocTraits::select_on_container_copy_construction(other.alloc)) {
        for (size_t i = 0; i < buckets.size(); ++i) {
            buckets[i].begin = elements.end();
            buckets[i].end = elements.end();
        }

        std::vector<NodeType> temp_elements;
        temp_elements.reserve(other.size());

        for (const auto& pair : other) {
            temp_elements.emplace_back(pair.first, pair.second);
        }

        for (const auto& element : temp_elements) {
            emplace(element.first, element.second);
        }
    }

    UnorderedMap(UnorderedMap&& other) noexcept
        : elements(std::move(other.elements)),
        buckets(std::move(other.buckets)),
        hasher(std::move(other.hasher)),
        key_equal(std::move(other.key_equal)),
        alloc(std::move(other.alloc))
    {}

    UnorderedMap& operator=(const UnorderedMap& other) {
        if (this == &other) {
            return *this;
        }

        if (NodeAllocTraits::propagate_on_container_copy_assignment::value) {
            if (alloc != other.alloc) {
                clear();
                alloc = other.alloc;
                elements = List<NodeType, NodeAlloc>(NodeAlloc(alloc));
                buckets = std::vector<Bucket, BucketAlloc>(other.buckets.size(), BucketAlloc(alloc));
                for (size_t i = 0; i < buckets.size(); ++i) {
                    buckets[i].begin = elements.end();
                    buckets[i].end = elements.end();
                }
            }
        }
        size_t size = static_cast<double>(other.size()) / max_load_factor();
        if (size > buckets.size()) {
            rehash(size);
        }

        std::vector<NodeType> temp_elements;
        temp_elements.reserve(other.size());

        for (const auto& pair : other) {
            temp_elements.emplace_back(pair.first, pair.second);
        }

        for (const auto& element : temp_elements) {
            emplace(element.first, element.second);
        }
        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        clear();
        if (NodeAllocTraits::propagate_on_container_move_assignment::value) {
            alloc = std::move(other.alloc);
        }
        elements = std::move(other.elements);
        buckets = std::move(other.buckets);
        hasher = std::move(other.hasher);
        key_equal = std::move(other.key_equal);
        return *this;
    }

    ~UnorderedMap() = default;

/*
-------------------------------------------------

          METHODS, GETTERS AND SETTERS

-------------------------------------------------
*/

    [[nodiscard]] size_t size() const noexcept {
        return elements.size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return elements.empty();
    }

    void clear() {
        elements.clear();
        for (size_t i = 0; i < buckets.size(); ++i) {
            buckets[i].begin = elements.end();
            buckets[i].end = elements.end();
        }
    }

    void swap(UnorderedMap& other) {
        std::swap(elements, other.elements);
        std::swap(buckets, other.buckets);
        std::swap(hasher, other.hasher);
        std::swap(key_equal, other.key_equal);
        if (NodeAllocTraits::propagate_on_container_swap::value) {
            std::swap(alloc, other.alloc);
        }
    }

/*
-------------------------------------------------

            REHASH AND HIS FRIENDS

-------------------------------------------------
*/

    void rehash(size_t new_size) {
        size_t min_required_buckets = static_cast<double>(size()) / max_load_factor();
        if (new_size < min_required_buckets) {
            new_size = min_required_buckets;
        }
        if (new_size == buckets.size() && !elements.empty()) {
            return;
        }

        buckets.assign(new_size, Bucket());
        for (size_t i = 0; i < new_size; ++i) {
            buckets[i].begin = elements.end();
            buckets[i].end = elements.end();
        }

        if (elements.empty()) {
            return;
        }

        std::vector<std::vector<typename List<NodeType, NodeAlloc>::iterator>> bucket_iters(new_size);

        for (auto it = elements.begin(); it != elements.end(); ++it) {
            size_t new_index = hasher(it->first) % new_size;
            bucket_iters[new_index].push_back(it);
        }

        auto current_pos = elements.begin();
        for (size_t bucket_index = 0; bucket_index < new_size; ++bucket_index) {
            if (!bucket_iters[bucket_index].empty()) {
                buckets[bucket_index].begin = current_pos;

                for (auto old_it : bucket_iters[bucket_index]) {
                    if (old_it != current_pos) {
                        current_pos = elements.move_node(old_it, current_pos);
                    }
                    ++current_pos;
                }

                buckets[bucket_index].end = current_pos;
            }
        }
    }

    [[nodiscard]] double load_factor() const noexcept {
        return static_cast<double>(size()) / buckets.size();
    }

    [[nodiscard]] double max_load_factor() const noexcept {
        return 1.0;
    }

    void reserve(size_t count) {
        size_t new_bucket_count = static_cast<double>(count) / max_load_factor();
        if (new_bucket_count == 0 && count > 0) {
            new_bucket_count = 1;
        }
        if (new_bucket_count > buckets.size()) {
            rehash(new_bucket_count);
        }
    }

/*
-------------------------------------------------

                     EMPLACE

-------------------------------------------------
*/

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        check_rehash();
        auto temp_it = elements.emplace(elements.end(), std::forward<Args>(args)...);
        const Key& newKey = temp_it->first;
        size_t index = get_bucket_index(newKey);

        for (auto it = buckets[index].begin; it != buckets[index].end; ++it) {
            if (key_equal(it->first, newKey)) {
                elements.erase(temp_it);
                return {iterator(it), false};
            }
        }

        if (buckets[index].begin == elements.end()) {
            buckets[index].begin = temp_it;
            buckets[index].end = std::next(temp_it);
            if (temp_it != elements.begin()) {
                auto prev_node = std::prev(temp_it);
                size_t prev_index = get_bucket_index(prev_node->first);
                if (prev_index != index && buckets[prev_index].end == elements.end()) {
                    buckets[prev_index].end = temp_it;
                }
            }
        }
        else {
            temp_it = elements.move_node(temp_it, buckets[index].end);
        }

        return {iterator(temp_it), true};
    }

/*
-------------------------------------------------

                    INSERT

-------------------------------------------------
*/

    std::pair<iterator, bool> insert(const NodeType& value) {
        check_rehash();
        size_t index = get_bucket_index(value.first);

        for (auto it = buckets[index].begin; it != buckets[index].end; ++it) {
            if (key_equal(it->first, value.first)) {
                return {iterator(it), false};
            }
        }

        typename List<NodeType, NodeAlloc>::iterator new_it;
        if (buckets[index].begin == elements.end()) {
            new_it = elements.emplace(elements.end(), value);
            buckets[index].begin = new_it;
            buckets[index].end = std::next(new_it);

            if (new_it != elements.begin()) {
                auto prev_node = std::prev(new_it);
                size_t prev_index = get_bucket_index(prev_node->first);
                if (prev_index != index && buckets[prev_index].end == elements.end()) {
                    buckets[prev_index].end = new_it;
                }
            }
        }
        else {
            new_it = elements.emplace(buckets[index].end, value);
        }
        return {iterator(new_it), true};
    }

    std::pair<iterator, bool> insert(NodeType&& value) {
        check_rehash();
        const Key& key = value.first;
        size_t index = get_bucket_index(key);

        for (auto it = buckets[index].begin; it != buckets[index].end; ++it) {
            if (key_equal(it->first, key)) {
                return {iterator(it), false};
            }
        }

        typename List<NodeType, NodeAlloc>::iterator new_it;
        if (buckets[index].begin == elements.end()) {
            new_it = elements.emplace(elements.end(), std::move(value));
            buckets[index].begin = new_it;
            buckets[index].end = std::next(new_it);

            if (new_it != elements.begin()) {
                auto prev_node = std::prev(new_it);
                size_t prev_index = get_bucket_index(prev_node->first);
                if (prev_index != index && buckets[prev_index].end == elements.end()) {
                    buckets[prev_index].end = new_it;
                }
            }
        }
        else {
            new_it = elements.emplace(buckets[index].end, std::move(value));
        }
        return {iterator(new_it), true};
    }

    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

/*
-------------------------------------------------

                     ERASE

-------------------------------------------------
*/

    iterator erase(const_iterator pos) {
        if (pos == end())
            return end();

        size_t index = get_bucket_index(pos->first);
        auto &bucket = buckets[index];

        using ConstIterator = typename List<NodeType, NodeAlloc>::const_iterator;
        ConstIterator it = pos.get_iterator();
        ConstIterator it_next = std::next(it);

        auto next_it = elements.erase(it);

        if (ConstIterator(bucket.begin) == it && ConstIterator(bucket.end) == it_next) {
            bucket.begin = bucket.end = elements.end();
        } else {
            if (ConstIterator(bucket.begin) == it)
                bucket.begin = next_it;
            if (ConstIterator(bucket.end) == it_next)
                bucket.end = next_it;
        }
        return iterator(next_it);
    }

    iterator erase(iterator pos) {
        return erase(const_iterator(pos));
    }

    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return iterator(typename List<NodeType, NodeAlloc>::iterator(last.get_iterator().node()));
    }

/*
-------------------------------------------------

                     FIND

-------------------------------------------------
*/

    iterator find(const Key& key) {
        size_t index = get_bucket_index(key);
        for (auto it = buckets[index].begin; it != buckets[index].end; ++it) {
            if (key_equal(it->first, key)) {
                return iterator(it);
            }
        }
        return end();
    }

    const_iterator find(const Key& key) const {
        size_t index = get_bucket_index(key);
        for (auto it = buckets[index].begin; it != buckets[index].end; ++it) {
            if (key_equal(it->first, key)) {
                return const_iterator(it);
            }
        }
        return end();
    }

/*
-------------------------------------------------

                 GET BY INDEX

-------------------------------------------------
*/

    Value& operator[](const Key& key) {
        auto [it, inserted] = emplace(key, Value());
        return it->second;
    }

    Value& operator[](Key&& key) {
        auto [it, inserted] = emplace(std::move(key), Value());
        return it->second;
    }

    Value& at(const Key& key) {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("Key not found");
        }
        return it->second;
    }

    const Value& at(const Key& key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("Key not found");
        }
        return it->second;
    }

/*
-------------------------------------------------

           CREATE UNORDERED MAP ITERATORS

-------------------------------------------------
*/

    iterator begin() noexcept {
        return iterator(elements.begin());
    }

    const_iterator begin() const noexcept {
        return const_iterator(elements.begin());
    }

    iterator end() noexcept {
        return iterator(elements.end());
    }

    const_iterator end() const noexcept {
        return const_iterator(elements.end());
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator cend() const noexcept {
        return end();
    }

/*
-------------------------------------------------

               BUCKET INTERFACE

-------------------------------------------------
*/

    size_t bucket_count() const noexcept {
        return buckets.size();
    }

    size_t bucket_size(size_t n) const {
        size_t count = 0;
        for (auto it = buckets[n].begin; it != buckets[n].end; ++it) {
            ++count;
        }
        return count;
    }

    iterator begin(size_t n) {
        return iterator(buckets[n].begin);
    }

    const_iterator begin(size_t n) const {
        return const_iterator(buckets[n].begin);
    }

    iterator end(size_t n) {
        return iterator(buckets[n].end);
    }

    const_iterator end(size_t n) const {
        return const_iterator(buckets[n].end);
    }

    const_iterator cbegin(size_t n) const {
        return begin(n);
    }

    const_iterator cend(size_t n) const {
        return end(n);
    }
};

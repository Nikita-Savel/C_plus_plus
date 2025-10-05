#pragma once

#include <memory>
#include <type_traits>

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

/*
=================================================

               BASE CONTROL BLOCK

=================================================
*/

struct BaseControlBlock {
  int shared_count = 0;
  int weak_count = 0;
  virtual ~BaseControlBlock() = default;
  virtual void destroy() noexcept = 0;
  virtual void deallocate() noexcept = 0;
  virtual void* get_object() noexcept = 0;
};

/*
=================================================

               SHARED PTR

=================================================
*/

template <typename T>
class SharedPtr {

/*
=================================================

            CONTROL BLOCK STANDARD

=================================================
*/

  template <typename Deleter, typename Alloc>
  struct ControlBlockStandard : BaseControlBlock {
    T* object = nullptr;
    Deleter deleter;
    Alloc alloc;

    template <typename D, typename A>
    ControlBlockStandard(T* p, D&& d, A&& a)
            : object(p),
              deleter(std::forward<D>(d)),
              alloc(std::forward<A>(a)) {
      this->shared_count = 1;
      this->weak_count = 0;
    }

    void destroy() noexcept override {
      deleter(object);
    }

    void deallocate() noexcept override {
      using BlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockStandard>;
      BlockAlloc block_alloc = alloc;
      std::destroy_at(this);
      block_alloc.deallocate(this, 1);
    }

    void* get_object() noexcept override {
      return object;
    }
  };

/*
=================================================

            CONTROL BLOCK MAKE SHARED

=================================================
*/

  template <typename Alloc, typename... Args>
  struct ControlBlockMakeShared : BaseControlBlock {
    T object;
    Alloc alloc;

    ControlBlockMakeShared(Alloc&& a, Args&&... args)
            : object(std::forward<Args>(args)...),
              alloc(std::move(a)) {
      this->shared_count = 1;
      this->weak_count = 0;
    }

    template <typename AllocArg, typename... Args2>
    ControlBlockMakeShared(AllocArg&& a, Args2&&... args)
            : object(std::forward<Args2>(args)...),
              alloc(std::forward<AllocArg>(a)) {
      this->shared_count = 1;
      this->weak_count = 0;
    }

    void destroy() noexcept override {
      std::allocator_traits<Alloc>::destroy(alloc, &object);
    }

    void deallocate() noexcept override {
      using BlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared>;
      BlockAlloc block_alloc(std::forward<Alloc>(alloc));
      alloc.~Alloc();
      block_alloc.deallocate(this, 1);
    }

    void* get_object() noexcept override {
      return &object;
    }
  };

  explicit SharedPtr(BaseControlBlock* control_block)
          : cb(control_block),
            ptr(static_cast<T*>(control_block->get_object())) {}

  template <typename Y>
  friend class SharedPtr;

  template <typename Y>
  friend class WeakPtr;

  friend class EnableSharedFromThis<T>;

  template <typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> constructSharedBlock(Alloc&& alloc, Args&&... args);

  template <typename U, typename... Args>
  friend SharedPtr<U> makeShared(Args&&... args);

  template <typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> allocateShared(Alloc&& alloc, Args&&... args);

 public:

/*
-------------------------------------------------

         CONSTRUCTORS AND DESTRUCTOR

-------------------------------------------------
*/

  SharedPtr() noexcept = default;

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  explicit SharedPtr(Y* my_ptr)
      : SharedPtr(my_ptr, std::default_delete<Y>(), std::allocator<Y>()) {}

  template <typename Y, typename Deleter, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  SharedPtr(Y* my_ptr, Deleter d)
      : SharedPtr(my_ptr, std::move(d), std::allocator<Y>()) {}

  template <typename Y, typename Deleter, typename Alloc, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  SharedPtr(Y* my_ptr, Deleter&& d, Alloc&& a) {
    using NonRefAlloc = std::remove_reference_t<Alloc>;
    using Block = ControlBlockStandard<std::decay_t<Deleter>, NonRefAlloc>;
    using BlockAlloc = typename std::allocator_traits<NonRefAlloc>::template rebind_alloc<Block>;

    BlockAlloc block_alloc(std::forward<Alloc>(a));

    try {
      cb = block_alloc.allocate(1);
      new (cb) Block(my_ptr, std::forward<Deleter>(d), std::forward<Alloc>(a));
      ptr = my_ptr;

      if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
        my_ptr->weak_this = *this;
      }
    } catch (...) {
      std::allocator_traits<BlockAlloc>::destroy(block_alloc, cb);
      block_alloc.deallocate(static_cast<Block*>(cb), 1);
      throw;
    }
  }

  template <typename Y>
  SharedPtr(const SharedPtr<Y>& other, T* alias_object) noexcept
          : cb(other.cb),
            ptr(alias_object) {
    if (cb) {
      ++cb->shared_count;
    }
  }

  SharedPtr(const SharedPtr& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    if (cb) {
      ++cb->shared_count;
    }
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  SharedPtr(const SharedPtr<Y>& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    if (cb) {
      ++cb->shared_count;
    }
  }

  SharedPtr(SharedPtr&& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    other.cb = nullptr;
    other.ptr = nullptr;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  SharedPtr(SharedPtr<Y>&& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    other.cb = nullptr;
    other.ptr = nullptr;
  }

  ~SharedPtr() {
    if (cb && --cb->shared_count == 0) {
      cb->destroy();
      if (cb->weak_count == 0) {
        cb->deallocate();
      }
    }
  }

/*
-------------------------------------------------

                  OPERATOR =

-------------------------------------------------
*/

  SharedPtr& operator=(const SharedPtr& other) noexcept {
    if (this != &other) {
      SharedPtr(other).swap(*this);
    }
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  SharedPtr& operator=(const SharedPtr<Y>& other) noexcept {
    SharedPtr(other).swap(*this);
    return *this;
  }

  SharedPtr& operator=(SharedPtr&& other) noexcept {
    if (this != &other) {
      SharedPtr(std::move(other)).swap(*this);
    }
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  SharedPtr& operator=(SharedPtr<Y>&& other) noexcept {
    SharedPtr(std::move(other)).swap(*this);
    return *this;
  }

/*
-------------------------------------------------

                OTHER METHODS

-------------------------------------------------
*/

  [[nodiscard]] int use_count() const noexcept {
    if (cb) {
      return cb->shared_count;
    }
    return 0;
  }

  T* get() const noexcept {
    return ptr;
  }

  operator bool() const noexcept {
    return ptr != nullptr;
  }

  void reset() noexcept {
    SharedPtr().swap(*this);
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  void reset(Y* my_ptr) {
    SharedPtr(my_ptr).swap(*this);
  }

  void swap(SharedPtr& other) noexcept {
    std::swap(cb, other.cb);
    std::swap(ptr, other.ptr);
  }

  T& operator*() const noexcept {
    return *ptr;
  }

  T* operator->() const noexcept {
    return ptr;
  }

  BaseControlBlock* cb = nullptr;
  T* ptr = nullptr;
};

/*
=================================================

                    WEAK PTR

=================================================
*/

template <typename T>
class WeakPtr {
  BaseControlBlock* cb = nullptr;
  T* ptr = nullptr;

  void increment_weak() {
    if (cb) {
      ++cb->weak_count;
    }
  }

  void decrement_weak() {
    if (cb) {
      if (--cb->weak_count == 0 && cb->shared_count == 0) {
        cb->deallocate();
      }
    }
  }

  template <typename Y>
  friend class SharedPtr;

  template <typename Y>
  friend class WeakPtr;

 public:

/*
-------------------------------------------------

           CONSTRUCTORS AND DESTRUCTOR

-------------------------------------------------
*/

  WeakPtr() = default;

  WeakPtr(const WeakPtr& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    increment_weak();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr(const WeakPtr<Y>& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    increment_weak();
  }

  WeakPtr(const SharedPtr<T>& sp) noexcept
          : cb(sp.cb),
            ptr(sp.ptr) {
    increment_weak();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr(const SharedPtr<Y>& sp) noexcept
          : cb(sp.cb),
            ptr(sp.ptr) {
    increment_weak();
  }

  WeakPtr(WeakPtr&& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    other.cb = nullptr;
    other.ptr = nullptr;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr(WeakPtr<Y>&& other) noexcept
          : cb(other.cb),
            ptr(other.ptr) {
    other.cb = nullptr;
    other.ptr = nullptr;
  }

  WeakPtr(SharedPtr<T>&& sp) noexcept
        : cb(sp.cb),
          ptr(sp.ptr) {
    sp.cb = nullptr;
    sp.ptr = nullptr;
    increment_weak();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr(SharedPtr<Y>&& sp) noexcept
          : cb(sp.cb),
            ptr(sp.ptr) {
    sp.cb = nullptr;
    sp.ptr = nullptr;
    increment_weak();
  }

  ~WeakPtr() {
    decrement_weak();
  }

/*
-------------------------------------------------

                  OPERATOR =

-------------------------------------------------
*/

  WeakPtr& operator=(const WeakPtr& other) noexcept {
    WeakPtr(other).swap(*this);
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr& operator=(const WeakPtr<Y>& other) noexcept {
    WeakPtr(other).swap(*this);
    return *this;
  }

  WeakPtr& operator=(const SharedPtr<T>& sp) noexcept {
    WeakPtr(sp).swap(*this);
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr& operator=(const SharedPtr<Y>& sp) noexcept {
    WeakPtr(sp).swap(*this);
    return *this;
  }

  WeakPtr& operator=(WeakPtr&& other) noexcept {
    WeakPtr(std::move(other)).swap(*this);
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr& operator=(WeakPtr<Y>&& other) noexcept {
    WeakPtr(std::move(other)).swap(*this);
    return *this;
  }

  WeakPtr& operator=(SharedPtr<T>&& sp) noexcept {
    WeakPtr(std::move(sp)).swap(*this);
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
  WeakPtr& operator=(SharedPtr<Y>&& sp) noexcept {
    WeakPtr(std::move(sp)).swap(*this);
    return *this;
  }

/*
-------------------------------------------------

                OTHER METHODS

-------------------------------------------------
*/

  [[nodiscard]] bool expired() const noexcept {
    return !cb || cb->shared_count == 0;
  }

  SharedPtr<T> lock() const noexcept {
    if (expired()) {
      return SharedPtr<T>();
    }
    SharedPtr<T> sp;
    sp.cb = cb;
    sp.ptr = ptr;
    ++cb->shared_count;
    return sp;
  }

  [[nodiscard]] int use_count() const noexcept {
    if (cb) {
      return cb->shared_count;
    }
    return 0;
  }

  void swap(WeakPtr& other) noexcept {
    std::swap(cb, other.cb);
    std::swap(ptr, other.ptr);
  }
};

/*
=================================================

             ENABLE SHARED FROM THIS

=================================================
*/

template <typename T>
class EnableSharedFromThis {
  WeakPtr<T> weak_this;

  template <typename U>
  friend class SharedPtr;

 public:
  SharedPtr<T> shared_from_this() {
    return weak_this.lock();
  }

  SharedPtr<const T> shared_from_this() const {
    return weak_this.lock();
  }
};

/*
------------------------------------------------

             MAKE / ALLOCATE SHARED

------------------------------------------------
*/

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> constructSharedBlock(Alloc&& alloc, Args&&... args) {
  using DecayedAlloc = std::decay_t<Alloc>;
  using Block = typename SharedPtr<T>::template ControlBlockMakeShared<DecayedAlloc, Args...>;
  using BlockAlloc = typename std::allocator_traits<DecayedAlloc>::template rebind_alloc<Block>;

  BlockAlloc block_alloc(alloc);
  auto* control_block = block_alloc.allocate(1);

  try {
    std::allocator_traits<BlockAlloc>::construct(
        block_alloc,
        control_block,
        std::forward<Alloc>(alloc),
        std::forward<Args>(args)...
    );
  } catch (...) {
    std::allocator_traits<BlockAlloc>::destroy(block_alloc, control_block);
    block_alloc.deallocate(static_cast<Block*>(control_block), 1);
    throw;
  }

  SharedPtr<T> sp;
  sp.cb = control_block;
  sp.ptr = static_cast<T*>(control_block->get_object());
  return sp;
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
  return constructSharedBlock<T>(
      std::allocator<T>{},
      std::forward<Args>(args)...
  );
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(Alloc&& alloc, Args&&... args) {
  return constructSharedBlock<T>(
      std::forward<Alloc>(alloc),
      std::forward<Args>(args)...
  );
}

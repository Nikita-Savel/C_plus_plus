#pragma once

static constexpr int BUFFER_SIZE = 16;

/*
=================================================

                 VIRTUAL TABLE

=================================================
*/

template<typename Ret, typename... Args>
struct VirtualTable {
  using invoke_ptr = Ret(*)(const void*, Args&&...);
  using copy_ptr = void(*)(void*, const void*);
  using move_ptr = void(*)(void*, void*);
  using destroy_ptr = void(*)(void*);
  using type_ptr = const std::type_info&(*)();
  using target_ptr = void*(*)(void*);

  invoke_ptr invoke;
  copy_ptr copy;
  move_ptr move;
  destroy_ptr destroy;
  type_ptr target_type;
  target_ptr get_target;
};

/*
=================================================

                SBO (STACK)

=================================================
*/

template<typename F, typename Ret, typename... Args>
struct SmallObject {
  static Ret invoke(const void* obj, Args&&... args) {
    F* obj_ptr = const_cast<F*>(reinterpret_cast<const F*>(obj));
    return std::invoke(*obj_ptr, std::forward<Args>(args)...);
  }

  static void copy(void* dest, const void* src) {
    if constexpr (std::is_copy_constructible_v<F>) {
      new (dest) F(*reinterpret_cast<const F*>(src));
    }
  }

  static void move(void* dest, void* src) {
    new (dest) F(std::move(*reinterpret_cast<F*>(src)));
    reinterpret_cast<F*>(src)->~F();
  }

  static void destroy(void* obj) {
    reinterpret_cast<F*>(obj)->~F();
  }

  static const std::type_info& target_type() {
    return typeid(F);
  }

  static void* get_target(void* obj) {
    return obj;
  }

  static constexpr VirtualTable<Ret, Args...> table = {
    &SmallObject::invoke,
    &SmallObject::copy,
    &SmallObject::move,
    &SmallObject::destroy,
    &SmallObject::target_type,
    &SmallObject::get_target
  };
};

/*
=================================================

                    HEAP

=================================================
*/

template<typename F, typename Ret, typename... Args>
struct BigObject {
  static Ret invoke(const void* obj, Args&&... args) {
    F* const* obj_const_ptr = reinterpret_cast<F* const*>(obj);
    return std::invoke(**obj_const_ptr, std::forward<Args>(args)...);
  }

  static void copy(void* dest, const void* src) {
    if constexpr (std::is_copy_constructible_v<F>) {
      F* const* src_const_ptr = reinterpret_cast<F* const*>(src);
      *reinterpret_cast<F**>(dest) = new F(**src_const_ptr);
    }
  }

  static void move(void* dest, void* src) {
    F** dest_ptr = reinterpret_cast<F**>(dest);
    F** src_ptr = reinterpret_cast<F**>(src);
    *dest_ptr = *src_ptr;
    *src_ptr = nullptr;
  }

  static void destroy(void* obj) {
    F** obj_ptr = reinterpret_cast<F**>(obj);
    if (*obj_ptr) {
      delete *obj_ptr;
    }
  }

  static const std::type_info& target_type() {
    return typeid(F);
  }

  static void* get_target(void* obj) {
    return obj;
  }

  static constexpr VirtualTable<Ret, Args...> table = {
    &BigObject::invoke,
    &BigObject::copy,
    &BigObject::move,
    &BigObject::destroy,
    &BigObject::target_type,
    &BigObject::get_target
  };
};

/*
=================================================

                  FUNCTION

=================================================
*/

template<bool IsMove, typename Ret>
class function;

template<bool IsMove, typename Ret, typename... Args>
class function<IsMove, Ret(Args...)> {
  using table = VirtualTable<Ret, Args...>;
  const table* table_ptr = nullptr;
  alignas(BUFFER_SIZE) char buffer[BUFFER_SIZE];

  void reset() noexcept {
    if (table_ptr) {
      table_ptr->destroy(buffer);
      table_ptr = nullptr;
    }
  }

  template<typename F>
  void assign(F&& f) {
    constexpr bool small_size = (sizeof(std::decay_t<F>) <= BUFFER_SIZE && alignof(std::decay_t<F>) <= BUFFER_SIZE);
    void* dest = buffer;
    if constexpr (small_size) {
      new (dest) std::decay_t<F>(std::forward<F>(f));
      table_ptr = &SmallObject<std::decay_t<F>, Ret, Args...>::table;
    } else {
      *reinterpret_cast<std::decay_t<F>**>(dest) = new std::decay_t<F>(std::forward<F>(f));
      table_ptr = &BigObject<std::decay_t<F>, Ret, Args...>::table;
    }
  }

public:

/*
-------------------------------------------------

          CONSTRUCTORS AND DESTRUCTOR

-------------------------------------------------
*/

  function() noexcept = default;

  template<typename F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, function> && std::is_invocable_r_v<Ret, F, Args...>>>
  function(F&& f) {
    assign(std::forward<F>(f));
  }

  function(const function& other) {
    if (other.table_ptr) {
      other.table_ptr->copy(buffer, other.buffer);
      table_ptr = other.table_ptr;
    }
  }

  function& operator=(const function& other) {
    if (this != &other) {
      reset();
      if (other.table_ptr) {
        other.table_ptr->copy(buffer, other.buffer);
        table_ptr = other.table_ptr;
      }
    }
    return *this;
  }

  function(function&& other) noexcept {
    if (other.table_ptr) {
      other.table_ptr->move(buffer, other.buffer);
      table_ptr = other.table_ptr;
      other.table_ptr = nullptr;
    }
  }

  ~function() {
    reset();
  }

/*
-------------------------------------------------

                 OPERATOR=

-------------------------------------------------
*/

  function& operator=(function&& other) noexcept {
    if (this != &other) {
      reset();
      if (other.table_ptr) {
        other.table_ptr->move(buffer, other.buffer);
        table_ptr = other.table_ptr;
        other.table_ptr = nullptr;
      }
    }
    return *this;
  }

  function& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  template<typename F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, function> && std::is_invocable_r_v<Ret, F, Args...>>>
  function& operator=(F&& f) {
    reset();
    assign(std::forward<F>(f));
    return *this;
  }

/*
-------------------------------------------------

            COMPARISON AND OPERATOR()

-------------------------------------------------
*/

  operator bool() const noexcept {
    return table_ptr != nullptr;
  }

  friend bool operator==(const function& f, std::nullptr_t) noexcept {
    return !f;
  }

  friend bool operator!=(const function& f, std::nullptr_t) noexcept {
    return f;
  }

  Ret operator()(Args... args) const {
    if (!table_ptr) {
      throw std::bad_function_call();
    }
    return table_ptr->invoke(buffer, std::forward<Args>(args)...);
  }

/*
-------------------------------------------------

                OTHER METHODS

-------------------------------------------------
*/

  const std::type_info& target_type() const noexcept {
    if (table_ptr->target_type()) {
      return table_ptr;
    }
    return typeid(void);
  }

  template<typename F>
  F* target() noexcept {
    if (table_ptr && table_ptr->target_type() == typeid(F)) {
      return reinterpret_cast<F*>(table_ptr->get_target(buffer));
    }
    return nullptr;
  }

  void swap(function& other) noexcept {
    function tmp(std::move(other));
    other = std::move(*this);
    *this = std::move(tmp);
  }
};

template<typename F>
struct function_traits;

template<typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)> {
  using type = Ret(Args...);
};

template<typename F, typename Ret, typename... Args>
struct function_traits<Ret(F::*)(Args...)> {
  using type = Ret(Args...);
};

template<typename F, typename Ret, typename... Args>
struct function_traits<Ret(F::*)(Args...) const> {
  using type = Ret(Args...);
};

template<typename F>
struct function_traits : function_traits<decltype(&F::operator())> {};

template<typename F>
function(F) -> function<false, typename function_traits<F>::type>;

template<typename F> using Function = function<false, F>;

template<typename F> using MoveOnlyFunction = function<true, F>;

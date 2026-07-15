# ⚙️ C++ — Data Structures & STL Implementations

Hands-on implementations of core C++ containers, algorithms and STL-like classes — written from scratch to practice memory management, templates and modern C++. Work is split across two university semesters.

---

## 🍂 C++ autumn 2024

| Module | What's inside |
| --- | --- |
| 🔢 **biginteger** | Arbitrary-precision integer (`BigInteger`) with arithmetic operators |
| 📐 **geometry** | 2D geometry primitives: points, lines, shapes |
| 🧮 **matrix** | Templated matrix class with linear-algebra operations |
| 📚 **stack** | Custom stack container |
| 🔤 **string** | Custom `String` class with dynamic memory management |
| ➕ **sum_multiply** | Arithmetic exercises |

## 🌱 C++ spring 2025

| Module | What's inside |
| --- | --- |
| 🔁 **circularbuffer** | Fixed-size ring buffer |
| 🎯 **function** | `std::function`-like type-erased callable wrapper |
| 🔗 **list + stackallocator** | Doubly-linked list with a custom stack allocator |
| 🧷 **smart_pointers** | `shared_ptr` / `unique_ptr`-style smart pointers |
| 🗺️ **unordered_map** | Hash table implementation of `unordered_map` |

---

## 🛠️ Highlights

- Manual memory management (allocators, RAII, rule of five)
- Templates & generic programming
- STL-compatible interfaces

## 🚀 Build & run

Each module contains its own sources. Compile with any C++17-compatible compiler, e.g.:

```bash
g++ -std=c++17 main.cpp -o main && ./main
```

#pragma once
#include <cstring>
#include <iostream>

class String {
  explicit String(const size_t count, const bool make_empty)
    : str_(new char[count + 1])
    , size_(count)
    , capacity_(count + 1) {
    if (make_empty) {
      size_ = 0;
    }
  }

  void IncreaseCapacity();

  char* str_ = new char[1];
  size_t size_ = 0;
  size_t capacity_ = 1;

public:
  String(const char* str);

  String(size_t count, char c);

  String() = default;

  String(const String& other);

  explicit String(size_t count);

  String& operator=(const String& other);

  ~String();

  const char& operator[](const size_t index) const {
    return str_[index];
  }

  char& operator[](const size_t index) {
    return str_[index];
  }

  size_t length() const {
    return size_;
  }

  size_t size() const {
    return size_;
  }

  size_t capacity() const {
    return capacity_ - 1;
  }

  void push_back(char c);

  void pop_back();

  const char& front() const {
    return str_[0];
  }

  char& front() {
    return str_[0];
  }

  const char& back() const {
    return str_[size_ - 1];
  }

  char& back() {
    return str_[size_ - 1];
  }

  String& operator+=(const String& other);

  String& operator+=(char c);

  String& operator+=(const char* str);

  size_t find(const String& substring) const;

  size_t rfind(const String& substring) const;

  String substr(size_t start, size_t count) const;

  bool empty() const {
    return size_ == 0;
  }

  void clear();

  void shrink_to_fit();

  const char* data() const {
    return str_;
  }

  char* data() {
    return str_;
  }
};

void String::IncreaseCapacity() {
  size_t new_capacity_ = capacity_ * 2 + 1;
  char* new_str = new char[new_capacity_];
  std::copy(str_, str_ + size_ + 1, new_str);

  delete[] str_;
  str_ = new_str;
  str_[size_] = '\0';
  capacity_ = new_capacity_;
}

String::String(const char* str)
    : String(strlen(str), false) {
  std::copy(str, str + capacity_, str_);
}

String::String(const size_t count, char c)
  : String(count, false) {
  memset(str_, c, count + 1);
  str_[size_] = '\0';
}

String::String(const String& other)
  : String(other.size_, false) {
  std::copy(other.str_, other.str_ + other.size_ + 1, str_);
}

String::String(size_t count)
  : String(count, true) {}

String& String::operator=(const String& other) {
  if (this != &other) {
    if (capacity_ <= other.size_) {
      delete[] str_;
      str_ = new char[other.capacity_];
      capacity_ = other.capacity_;
    }
    size_ = other.size_;
    std::copy(other.str_, other.str_ + other.size_ + 1, str_);
  }
  return *this;
}

String::~String() {
  delete[] str_;
}

void String::push_back(char c) {
  if (size_ >= capacity_ - 1) {
    IncreaseCapacity();
  }
  str_[size_] = c;
  ++size_;
  str_[size_] = '\0';
}

void String::pop_back() {
  if (size_ > 0) {
    str_[size_ - 1] = '\0';
    size_--;
  }
}

String& String::operator+=(const String& other) {
  const size_t kNewSize = size_ + other.size_;
  if (kNewSize >= capacity_ - 1) {
    capacity_ = kNewSize / 2;
    IncreaseCapacity();
  }
  std::copy(other.str_, other.str_ + other.size_ + 1, str_ + size_);
  size_ = kNewSize;

  return *this;
}

String& String::operator+=(char c) {
  push_back(c);
  return *this;
}

String& String::operator+=(const char* str) {
  std::copy(str, str + strlen(str) + 1, str_ + size_);
  size_ += strlen(str);
  return *this;
}

size_t String::find(const String& substring) const {
  if (substring.size() > size()) {
    return length();
  }
  for (size_t i = 0; i <= size_ - substring.size_; ++i) {
    if (memcmp(str_ + i, substring.str_, substring.size_) == 0) {
      return i;
    }
  }
  return length();
}

size_t String::rfind(const String& substring) const {
  if (substring.size() > size()) {
    return length();
  }
  for (size_t i = size_ - substring.size_ + 1; i--;)  {
    if (memcmp(str_ + i, substring.str_, substring.size_) == 0) {
      return i;
    }
  }
  return length();
}

String String::substr(size_t start, size_t count) const {
  String substring(std::min(count, size_ - start), '\0');
  if (start + count > size_) {
    std::copy(str_ + start, str_ + size_, substring.str_);
    return substring;
  }
  std::copy(str_ + start, str_ + start + count, substring.str_);
  return substring;
}

void String::clear() {
  size_ = 0;
  str_[0] = '\0';
}

void String::shrink_to_fit() {
  if (size_ + 1 != capacity_) {
    capacity_ = size_ / 2;
    IncreaseCapacity();
  }
}

bool operator==(const String& a, const String& b) {
  return memcmp(a.data(), b.data(), std::min(a.size(), b.size())) == 0;
}

bool operator!=(const String& a, const String& b) {
  return !(a == b);
}

bool operator<(const String& a, const String& b) {
  return memcmp(a.data(), b.data(), std::min(a.size(), b.size())) > 0;
}

bool operator<=(const String& a, const String& b) {
  return a < b || a == b;
}

bool operator>(const String& a, const String& b) {
  return !(a <= b);
}


bool operator>=(const String& a, const String& b) {
  return !(a < b);
}

String operator+(const String& str, char c) {
  String result(str.size() + 1);
  result += str;
  result += c;
  return result;
}

String operator+(char c, const String& str) {
  String result(str.size() + 1);
  result += c;
  result += str;
  return result;
}

String operator+(const String& str1, const String& str2) {
  String result(str1.size() + str2.size());
  result += str1;
  result += str2;
  return result;
}

String operator+(const char* str1, const String& str2) {
  String result(strlen(str1) + str2.size());
  result += str1;
  result += str2;
  return result;
}

String operator+(const String& str1, const char* str2) {
  String result(str1.size() + strlen(str2));
  result += str1;
  result += str2;
  return result;
}

std::ostream& operator<<(std::ostream& out, const String& string) {
  for (size_t i = 0; i < string.size(); ++i) {
    out << string[i];
  }
  return out;
}

std::istream& operator>>(std::istream& in, String& string) {
  string.clear();
  char symbol;
  in.get(symbol);
  while (!in.eof() && !std::isspace(symbol)) {
    string.push_back(symbol);
    symbol = static_cast<char>(in.get());
  }
  return in;
}

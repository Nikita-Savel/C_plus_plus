#pragma once

#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cmath>
#include <sstream>
#include <string>

class BigInteger {
  const static int kBase = 1e9;
  std::vector<int64_t> digits_;
  bool is_positive_;

public:
  std::vector<int64_t> getDigits() const {
    return digits_;
  }

  bool getIsPositive() const {
    return is_positive_;
  }

  void setIsPositive(bool new_is_positive) {
    is_positive_ = new_is_positive;
  }

  BigInteger(int64_t number) {
    if (number >= 0) {
      is_positive_ = true;
    } else {
      is_positive_ = false;
      number = -number;
    }
    if (number == 0) {
      digits_.push_back(0);
    }
    while (number > 0) {
      digits_.push_back(number % kBase);
      number /= kBase;
    }
  }

  BigInteger(const BigInteger& other) : digits_(other.digits_), is_positive_(other.is_positive_) {}

  BigInteger(const std::string& str) {
    int start = 0;
    is_positive_ = true;
    if (str[0] == '-') {
      start = 1;
      if (str[1] != '0') {
        is_positive_ = false;
      }
    }

    int current = 0;
    int flag = str.size();
    for (int i = str.size() - 9; i > start; i -= 9) {
      flag -= 9;
      current = 0;
      for (int j = i; j <= i + 8; j++) {
        current *= 10;
        current += str[j] - '0';
      }
      digits_.push_back(current);
    }
    current = 0;
    for (int j = start; j < flag; j++) {
      current *= 10;
      current += str[j] - '0';
    }
    if (current != 0) {
      digits_.push_back(current);
    } else {
      digits_.push_back(0);
    }
  }

  BigInteger(): BigInteger(0) {}

  ~BigInteger() = default;

// --------------------------------------------------------------

  std::string toString() const;

// --------------------------------------------------------------

  BigInteger operator-() const;

  bool operator==(const BigInteger& other) const;

  BigInteger& operator+=(const BigInteger& other);

  BigInteger& operator-=(const BigInteger& other);

  BigInteger& operator*=(const BigInteger& other);

  BigInteger& operator/=(const BigInteger& other);

  BigInteger& operator%=(const BigInteger& other);

  BigInteger& operator=(const BigInteger& other) = default;

// --------------------------------------------------------------

  BigInteger& operator++();

  BigInteger& operator--();

  BigInteger operator++(int);

  BigInteger operator--(int);

// --------------------------------------------------------------

  explicit operator bool() {
    return !(*this == 0);
  }

  int checkModule(const BigInteger& other) const {
    if (digits_.size() > other.digits_.size()) {
      return 1;
    }
    if (digits_.size() < other.digits_.size()) {
      return -1;
    }

    for (int i = digits_.size() - 1; i >= 0; --i) {
      if (digits_[i] < other.digits_[i]) {
        return -1;
      }
      if (digits_[i] > other.digits_[i]) {
        return 1;
      }
    }
    return 0;
  }

  BigInteger& sum(const BigInteger& other) {
    const int kBase = 1e9;
    size_t max_size = std::max(digits_.size(), other.digits_.size());
    digits_.resize(max_size + 1, 0);

    for (size_t i = 0; i < max_size; ++i) {
      if (i < other.digits_.size()) {
        digits_[i] += other.digits_[i];
      }
      if (digits_[i] >= kBase) {
        digits_[i] -= kBase;
        digits_[i + 1] += 1;
      }
    }

    while (digits_.size() > 1 && digits_.back() == 0) {
      digits_.pop_back();
    }
    if ((*this).isZero()) {
      is_positive_ = true;
    }
    return *this;
  }

  BigInteger& subtract(const BigInteger& other) {
    size_t max_size = std::max(digits_.size(), other.digits_.size());
    digits_.resize(max_size, 0);

    for (size_t i = 0; i < max_size; ++i) {
      if (i < other.digits_.size()) {
        digits_[i] -= other.digits_[i];
      }

      if (digits_[i] < 0) {
        digits_[i] += kBase;
        digits_[i + 1] -= 1;
      }
    }

    while (digits_.size() > 1 && digits_.back() == 0) {
      digits_.pop_back();
    }
    if ((*this).isZero()) {
      is_positive_ = true;
    }
    return *this;
  }

  bool isZero() const {
  	return digits_.size() == 1 && digits_[0] == 0;
  }
};

// --------------------------------------------------------------
// --------------------------------------------------------------

BigInteger operator ""_bi(unsigned long long number) {
  return BigInteger(number);
}

BigInteger operator ""_bi(const char* str) {
  return BigInteger(std::string(str));
}

BigInteger operator ""_bi(const char* str, size_t size) {
  return BigInteger(std::string(str, size));
}

// --------------------------------------------------------------

std::string BigInteger::toString() const {
  if ((*this).isZero()) {
    return "0";
  }
  std::string str;
  if (!is_positive_) {
    str += "-";
  }
  str += std::to_string(digits_.back());
  for (int i = static_cast<int> (digits_.size()) - 2; i >= 0; --i) {
    std::string block = std::to_string(digits_[i]);
    str += std::string(9 - block.length(), '0') + block;
  }
  return str;
}

// --------------------------------------------------------------

bool BigInteger::operator==(const BigInteger& other) const {
  return is_positive_ == other.is_positive_ && digits_ == other.digits_;
}

bool operator<(const BigInteger& a, const BigInteger& b) {
  if (a.getIsPositive() != b.getIsPositive()) {
    return !a.getIsPositive();
  }
  if (a.getIsPositive()) {
    return a.checkModule(b) == -1;
  }
  return b.checkModule(a) == -1;
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
  if ((*this).isZero()) {
    *this = other;
    return *this;
  }
  if (other.isZero()) {
    return *this;
  }

  if (is_positive_ == other.is_positive_) {
    (*this).sum(other);
  } else {
    if ((*this).checkModule(other) >= 0) {
      (*this).subtract(other);
    } else {
      BigInteger copy = other;
      copy.subtract(*this);
      (*this) = copy;
    }
  }
  return *this;
}

bool operator!=(const BigInteger& a, const BigInteger& b) {
  return !(a == b);
}

BigInteger& BigInteger::operator-=(const BigInteger& other) {
  if (*this == other) {
    *this = BigInteger();
    return *this;
  }
  if (!(*this).isZero()) {
    (*this).is_positive_ = !(*this).is_positive_;
  }
  (*this) += other;
  if (!(*this).isZero()) {
    (*this).is_positive_ = !(*this).is_positive_;
  } else {
    (*this).is_positive_ = true;
  }
  return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& other) {
  bool new_is_positive_ = true;
  if (is_positive_ != other.is_positive_) {
    new_is_positive_ = false;
  }
  std::vector<int64_t> result(digits_.size() + other.digits_.size(), 0);
//  result.reserve(digits_.size());
  int carry = 0;
  for (size_t i = 0; i < digits_.size(); ++i) {
    carry = 0;
    for (size_t j = 0; j < other.digits_.size(); ++j) {
      int64_t product = digits_[i] * other.digits_[j] + result[i + j] + carry;
      result[i + j] = product % kBase;
      carry = product / kBase;
    }
    result[i + other.digits_.size()] += carry;
  }

  while (result.back() == 0 && result.size() > 1) {
    result.pop_back();
  }
  digits_ = std::move(result);
  is_positive_ = new_is_positive_;
  if ((*this).isZero()) {
    is_positive_ = true;
  }
  return *this;
}

// --------------------------------------------------------------

BigInteger& BigInteger::operator++() {
  *this += "1"_bi;
  return *this;
}

BigInteger& BigInteger::operator--() {
  *this -= "1"_bi;
  return *this;
}

BigInteger BigInteger::operator++(int) {
  BigInteger temp = *this;
  *this += "1"_bi;
  return temp;
}

BigInteger BigInteger::operator--(int) {
  BigInteger temp = *this;
  *this -= "1"_bi;
  return temp;
}

// --------------------------------------------------------------
// --------------------------------------------------------------

BigInteger operator+(const BigInteger& first, const BigInteger& second) {
  BigInteger result = first;
  result += second;
  return result;
}

BigInteger operator-(const BigInteger& first, const BigInteger& second) {
  BigInteger result = first;
  result -= second;
  return result;
}

BigInteger operator*(const BigInteger& first, const BigInteger& second) {
  BigInteger result = first;
  result *= second;
  return result;
}

BigInteger operator/(const BigInteger& first, const BigInteger& second) {
  if (second.isZero()) {
    throw std::invalid_argument("Division by 0");
  }
  BigInteger result = first;
  result /= second;
  return result;
}

BigInteger operator%(const BigInteger& first, const BigInteger& second) {
  if (second.isZero()) {
    throw std::invalid_argument("Division by 0");
  }
  BigInteger result = first;
  result %= second;
  return result;
}

// --------------------------------------------------------------

bool operator>(const BigInteger& a, const BigInteger& b) {
  return (b < a);
}

bool operator>=(const BigInteger& a, const BigInteger& b) {
  return !(a < b);
}

bool operator<=(const BigInteger& a, const BigInteger& b) {
  return !(a > b);
}

// --------------------------------------------------------------

int64_t binSearch(int64_t left, int64_t right, const BigInteger& first, const BigInteger& second) {
  while (left + 1 < right) {
    const int64_t median = (left + right) / 2;
    BigInteger product = first * median;

    if (product == second) {
      return median;
    }
    if (product < second) {
      left = median;
    } else {
      right = median;
    }
  }
  return left;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
  if (other.isZero()) {
    throw std::invalid_argument("Division by 0");
  }
  BigInteger nother = other;
  bool new_is_positive_ = (is_positive_ == nother.is_positive_);
  nother.is_positive_ = true;

  if (checkModule(nother) == -1) {
    *this = 0;
    return *this;
  }
  std::vector<int64_t> result;
  result.reserve(digits_.size());
  BigInteger mod;
  for (size_t i = digits_.size(); i > 0; --i) {
    mod = mod * kBase + BigInteger(digits_[i - 1]);
    if (mod < nother) {
      result.push_back(0);
      continue;
    }
    int64_t count = binSearch(0, 1e9, nother, mod);
    result.push_back(count);
    mod -= BigInteger(count) * nother;
  }
  std::reverse(result.begin(), result.end());

  while (result.size() > 1 && result.back() == 0) {
    result.pop_back();
  }

  digits_ = std::move(result);
  is_positive_ = new_is_positive_;

  if (digits_.empty()) {
    digits_.push_back(0);
    is_positive_ = true;
  }
  if ((*this).isZero()) {
    is_positive_ = true;
  }
  return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& other) {
  if (other.isZero()) {
    throw std::invalid_argument("Division by 0");
  }
  *this -= (*this / other) * other;
  return *this;
}

// --------------------------------------------------------------

BigInteger BigInteger::operator-() const {
  BigInteger result = *this;
  if (!result.isZero()) {
    result.is_positive_ = !result.is_positive_;
  }
  return result;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& big_integer) {
  out << big_integer.toString();
  return out;
}

std::istream& operator>>(std::istream& in, BigInteger& big_integer) {
  std::string str;
  in >> str;
  big_integer = BigInteger(str);
  return in;
}

// --------------------------------------------------------------

class Rational {
  BigInteger numerator_;
  BigInteger denominator_;
  static const int kNumbersAfterPoint = 50;

public:
  Rational(const int numerator, const int denominator) : numerator_(numerator), denominator_(denominator) {
    normalize();
  }

  Rational(const BigInteger& numerator, const BigInteger& denominator) : numerator_(numerator), denominator_(denominator) {
    normalize();
  }

  Rational(const Rational& other) = default;

  Rational(int number) : numerator_(BigInteger(number)), denominator_(BigInteger(1)) {}

  Rational(BigInteger number) : numerator_(number), denominator_(BigInteger(1)) {}

  Rational() = default;

  ~Rational() = default;

// --------------------------------------------------------------

  std::string toString() const;

  std::string asDecimal(size_t precision = 0) const;

// --------------------------------------------------------------

  Rational operator-() const;

  Rational& operator+=(const Rational& other);

  Rational& operator-=(const Rational& other);

  Rational& operator*=(const Rational& other);

  Rational& operator/=(const Rational& other);

  friend bool operator==(const Rational& a, const Rational& b);
  friend bool operator>(const Rational& a, const Rational& b);

// --------------------------------------------------------------

  explicit operator double() const {
    return stod(asDecimal(kNumbersAfterPoint));
  }

// --------------------------------------------------------------

private:
  BigInteger gcd(BigInteger first, BigInteger second) {
    first.setIsPositive(true);
    second.setIsPositive(true);
    while (!second.isZero()) {
      BigInteger temp = second;
      second = first % second;
      first = temp;
    }
    return first;
  }

  void normalize() {
    if (numerator_.isZero()) {
      denominator_ = "1"_bi;
      return;
    }
    BigInteger current_gcd = gcd(numerator_, denominator_);
    numerator_ /= current_gcd;
    denominator_ /= current_gcd;
    if (!denominator_.getIsPositive()) {
      numerator_.setIsPositive(!numerator_.getIsPositive());
      denominator_.setIsPositive(true);
    }
  }
};

// --------------------------------------------------------------
// --------------------------------------------------------------

std::string Rational::toString() const {
  if (denominator_ == "1"_bi) {
    return numerator_.toString();
  }
  return numerator_.toString() + "/" + denominator_.toString();
}

std::string Rational::asDecimal(const size_t precision) const {
  BigInteger numerator = numerator_;
  BigInteger denominator = denominator_;
  bool is_negative = !numerator_.getIsPositive();
  numerator.setIsPositive(true);
  std::ostringstream oss;
  if (is_negative) {
    oss << '-';
  }
  oss << (numerator / denominator).toString();
  if (denominator != 1 && precision != 0) {
    oss << '.';
    for (size_t i = 0; i < precision; ++i) {
      numerator %= denominator;
      numerator *= 10;
      oss << (numerator / denominator).toString();
    }
  }
  return oss.str();
}

// --------------------------------------------------------------

Rational& Rational::operator+=(const Rational& other) {
  numerator_ = numerator_ * other.denominator_ + other.numerator_ * denominator_;
  denominator_ *= other.denominator_;
  normalize();
  return *this;
}

Rational& Rational::operator-=(const Rational& other) {
  numerator_ = numerator_ * other.denominator_ - other.numerator_ * denominator_;
  denominator_ *= other.denominator_;
  normalize();
  return *this;
}

Rational& Rational::operator*=(const Rational& other) {
  numerator_ *= other.numerator_;
  denominator_ *= other.denominator_;
  normalize();
  return *this;
}

Rational& Rational::operator/=(const Rational& other) {
  numerator_ *= other.denominator_;
  denominator_ *= other.numerator_;
  normalize();
  return *this;
}

// --------------------------------------------------------------

Rational Rational::operator-() const {
  Rational result = *this;
  result.numerator_.setIsPositive(!result.numerator_.getIsPositive());
  return result;
}

Rational operator+(const Rational& first, const Rational& second) {
  Rational result = first;
  result += second;
  return result;
}

Rational operator-(const Rational& first, const Rational& second) {
  Rational result = first;
  result -= second;
  return result;
}

Rational operator*(const Rational& first, const Rational& second) {
  Rational result = first;
  result *= second;
  return result;
}

Rational operator/(const Rational& first, const Rational& second) {
  Rational result = first;
  result /= second;
  return result;
}

// --------------------------------------------------------------

bool operator==(const Rational& a, const Rational& b) {
  return a.numerator_ == b.numerator_ && a.denominator_ == b.denominator_;
}

bool operator!=(const Rational& a, const Rational& b) {
  return !(a == b);
}

bool operator>(const Rational& a, const Rational& b) {
  return a.numerator_ * b.denominator_ > b.numerator_ * a.denominator_;
}

bool operator<(const Rational& a, const Rational& b) {
  return (b > a);
}

bool operator>=(const Rational& a, const Rational& b) {
  return !(a < b);
}

bool operator<=(const Rational& a, const Rational& b) {
  return !(a > b);
}

std::ostream& operator<<(std::ostream& out, const Rational& big_integer) {
  out << big_integer.toString();
  return out;
}

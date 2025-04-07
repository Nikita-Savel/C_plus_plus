#include <array>
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

std::istream& operator>>(std::istream& in, Rational& rational) {
  std::string str;
  in >> str;
  int pos_dot = str.find('/');
  if (pos_dot != -1) {
    rational = Rational(str.substr(0, pos_dot), str.substr(pos_dot + 1));
  } else {
    rational = Rational(str);
  }
  return in;
}

/*
============================================================

                         MATRIX

============================================================
*/

constexpr bool is_prime(size_t n) {
  if (n <= 0) {
    return false;
  }
  if (n % 2 == 0) {
    return false;
  }
  for (size_t i = 3; i * i <= n; i += 2) {
    if (n % i == 0) {
      return false;
    }
  }
  return true;
}

template<size_t N>
class Residue {
  size_t extended_gcd(size_t num1, size_t num2, int& ans1, int& ans2) {
    if (num1 == 0) {
      ans1 = 0;
      ans2 = 1;
      return num2;
  	}
 	int tmp1;
    int tmp2;
    size_t ans = extended_gcd(num2 % num1, num1, tmp1, tmp2);
    ans1 = tmp2 - (num2 / num1) * tmp1;
    ans2 = tmp1;
    return ans;
  }

public:
  size_t value_;

  Residue() : value_(0) {}

  Residue(int value) {
    if (value >= 0) {
      value_ = value % N;
    } else {
      value_ = (N - (std::abs(static_cast<int64_t> (value)) % N)) % N;
    }
  }

  Residue(const Residue<N>& residue) = default;

  explicit operator int() const {
    return static_cast<int>(value_);
  }

  Residue<N>& operator=(const Residue<N>& other) = default;

  Residue<N>& operator+=(const Residue<N> other) {
    value_ = (value_ + other.value_) % N;
    return *this;
  }

  Residue<N>& operator-=(const Residue<N> other) {
    value_ = (value_ - other.value_ + N) % N;
    return *this;
  }

  Residue<N>& operator*=(const Residue<N> other) {
    value_ = (value_ * other.value_) % N;
    return *this;
  }

  Residue<N>& operator/=(const Residue<N> other) {
    static_assert(is_prime(N), "Division is not supported for composite N");
    int ans1 = 0;
    int ans2 = 0;
    int mod = N;
    extended_gcd(other.value_, mod, ans1, ans2);
    value_ *= (ans1 % mod + mod) % mod;
    value_ %= mod;
    return *this;
  }
};

template <size_t N>
bool operator==(const Residue<N> residue1, const Residue<N> residue2) {
  return residue1.value_ == residue2.value_;
}

template <size_t N>
bool operator!=(const Residue<N> residue1, const Residue<N> residue2) {
  return !(residue1 == residue2);
}

template <size_t N>
bool operator==(const Residue<N> residue1, int64_t value) {
  return residue1.value_ == value;
}

template <size_t N>
bool operator!=(const Residue<N> residue1, int64_t value) {
  return residue1.value_ != value;
}

template <size_t N>
Residue<N> operator+(const Residue<N> residue1, const Residue<N> residue2) {
  Residue<N> result = residue1;
  result += residue2;
  return result;
}

template <size_t N>
Residue<N> operator-(const Residue<N> residue1, const Residue<N> residue2) {
  Residue<N> result = residue1;
  result -= residue2;
  return result;
}

template <size_t N>
Residue<N> operator*(const Residue<N> residue1, const Residue<N> residue2) {
  Residue<N> result = residue1;
   result *= residue2;
  return result;
}

template <size_t N>
Residue<N> operator/(const Residue<N> residue1, const Residue<N> residue2) {
  Residue<N> result = residue1;
  result /= residue2;
  return result;
}

template <size_t N>
std::ostream& operator<<(std::ostream& out, const Residue<N> residue) {
  out << residue.value_;
  return out;
}

template <size_t N>
std::istream& operator>>(std::istream& is, const Residue<N> residue) {
  size_t tmp;
  is >> tmp;
  residue.value_ = tmp % N;
  return is;
}

//-------------------------------------------------------

template <size_t M, size_t N, typename Field = Rational>
class Matrix {
public:
  std::array<std::array<Field, N>, M> data_;

  Matrix() {
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        data_[i][j] = Field(0);
      }
    }
  }

  Matrix(const Matrix<M, N, Field>& matrix) = default;

  Matrix(const std::initializer_list<std::initializer_list<Field>>& list) {
    assert(list.size() == M && "Initializer_list size does not match matrix dimensions");
    size_t i = 0;
    for (const auto& row : list) {
      assert(row.size() == N && "Initializer_list size does not match matrix dimensions");
      std::copy(row.begin(), row.end(), data_[i].begin());
      ++i;
    }
  }

  static Matrix<M, N, Field> unityMatrix() {
    static_assert(M == N, "Unity matrix can only be created for square matrices");
    Matrix<M, N, Field> result;
    for (size_t i = 0; i < M; ++i) {
      result.data_[i][i] = Field(1);
    }
    return result;
  }

  Matrix<M, N, Field>& operator=(const Matrix<M, N, Field>& other) = default;

  Matrix<M, N, Field>& operator+=(const Matrix<M, N, Field>& matrix) {
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        data_[i][j] += matrix.data_[i][j];
      }
    }
    return *this;
  }

  Matrix<M, N, Field>& operator-=(const Matrix<M, N, Field>& matrix) {
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        data_[i][j] -= matrix.data_[i][j];
      }
    }
    return *this;
  }

  Matrix<M, N, Field>& operator*=(const Field& scalar) {
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        data_[i][j] *= scalar;
      }
    }
    return *this;
  }

  Matrix<M, M, Field>& operator*=(const Matrix<N, N, Field>& matrix);

  Matrix<N, M, Field> transposed() const {
    Matrix<N, M, Field> transpose_matrix;
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        transpose_matrix.data_[j][i] = data_[i][j];
      }
    }
    return transpose_matrix;
  }

  Field det() const {
    static_assert(M == N, "Determinant can only be calculated for square matrices");
    Matrix<M, N, Field> temp = *this;
    Field mod = Field(1);
    size_t ans = 0;
    for (size_t col = 0; col < M; ++col) {
      size_t pivot_row = ans;
      for (size_t j = ans + 1; j < M; ++j) {
        if (temp[j][col] != Field(0)) {
          pivot_row = j;
        }
      }
      if (temp[pivot_row][col] == Field(0)) {
        continue;
      } else {
        if (ans + 1 == M) {
          ++ans;
          break;
        }
        for (size_t j = 0; j < M; ++j) {
          std::swap(temp[pivot_row][j], temp[ans][j]);
		}
        if (ans != pivot_row) {
          mod *= Field(-1);
        }
        ans += 1;
      }
      for (size_t i = ans; i < M; ++i) {
        Field factor = temp[i][col] / temp[ans - 1][col];
        for (size_t j = col; j < N; ++j) {
          temp[i][j] -= factor * temp[ans - 1][j];
        }
      }
    }
    Field det = Field(1);
    for (size_t i = 0; i < M; i++) {
      det *= temp[i][i];
    }
    return det * mod;
  }

  size_t rank() const {
    Matrix<M, N, Field> temp = *this;
    size_t rank = 0;
    for (size_t col = 0; col < N; ++col) {
      size_t pivot_row = rank;
      while (pivot_row < M && temp.data_[pivot_row][col] == Field(0)) {
        pivot_row++;
      }
      if (pivot_row == M) {
        continue;
      }
      if (pivot_row != rank) {
        std::swap(temp.data_[rank], temp.data_[pivot_row]);
      }
      for (size_t j = 0; j < N; ++j) {
        if (j != col) {
          temp.data_[rank][j] /= temp.data_[rank][col];
        }
      }
      temp.data_[rank][col] = 1;

      for (size_t i = rank + 1; i < M; ++i) {
        Field factor = temp.data_[i][col];
        for (size_t j = 0; j < N; ++j) {
          temp.data_[i][j] -= factor * temp.data_[rank][j];
        }
      }
      rank++;
    }
    return rank;
  }

  std::array<Field, N>& operator[](size_t index) {
    assert(index < M && "Index out of bounds");
    return data_[index];
  }

  const std::array<Field, N>& operator[](size_t index) const {
    assert(index < M && "Index out of bounds");
    return data_[index];
  }

  Matrix<M, 2 * N, Field> createGausseMatrix() const {
    Matrix<M, 2 * N, Field> gausse_matrix;
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        gausse_matrix.data_[i][j] = data_[i][j];
      }
      for (size_t j = N; j < 2 * N; ++j) {
        if (j - N == i) {
          gausse_matrix.data_[i][j] = Field(1);
        } else {
          gausse_matrix.data_[i][j] = Field(0);
        }
      }
    }
    return gausse_matrix;
  }

  static Matrix<M, N, Field> extractInverse(const Matrix<M, 2 * N, Field>& augmented) {
    Matrix<M, N, Field> inverse;
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        inverse.data_[i][j] = augmented.data_[i][j + N];
      }
    }
    return inverse;
  }

  void invert() {
    static_assert(M == N, "Inversion can only be performed on square matrices");
	Matrix<M, 2 * N, Field> gausse_matrix = createGausseMatrix();
    for (size_t i = 0; i < N; ++i) {
      Field pivot = gausse_matrix.data_[i][i];
      assert(pivot != Field(0) && "The inverse matrix can't be defined for matrices with zero determinant");
      for (size_t j = 0; j < 2 * N; ++j) {
        gausse_matrix.data_[i][j] /= pivot;
      }
      for (size_t k = 0; k < N; ++k) {
        if (k != i) {
          Field factor = gausse_matrix.data_[k][i];
          for (size_t j = 0; j < 2 * N; ++j) {
            gausse_matrix.data_[k][j] -= factor * gausse_matrix.data_[i][j];
          }
        }
      }
    }
    *this = extractInverse(gausse_matrix);
  }

  Matrix<M, N, Field> inverted() const {
    Matrix<M, N, Field> temp = *this;
    temp.invert();
    return temp;
  }

  Field trace() const {
    static_assert(M == N, "Trace can only be taken from a square matrix");
    Field trace = Field(0);
    for (size_t i = 0; i < M; ++i) {
      trace += data_[i][i];
    }
    return trace;
  }

  std::array<Field, N> getRow(unsigned index) {
    assert(index < M && "Row index out of bounds");
    return data_[index];
  }

  std::array<Field, M> getColumn(unsigned index) {
    assert(index < N && "Column index out of bounds");
    std::array<Field, M> column;
    for (size_t i = 0; i < M; ++i) {
      column[i] = data_[i][index];
    }
    return column;
  }
};

template <size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator+(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
  Matrix<M, N, Field> result = matrix1;
  return result += matrix2;
}

template <size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator-(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
  Matrix<M, N, Field> result = matrix1;
  return result -= matrix2;
}

template <size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator*(const Matrix<M, N, Field>& matrix, const Field& scalar) {
  Matrix<M, N, Field> result = matrix;
  return result *= scalar;
}

template <size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator*(const Field& scalar, const Matrix<M, N, Field>& matrix) {
  Matrix<M, N, Field> result = matrix;
  return result *= scalar;
}

template <size_t M, size_t N, size_t P, typename Field = Rational>
Matrix<M, P, Field> operator*(const Matrix<M, N, Field>& matrix1, const Matrix<N, P, Field>& matrix2) {
  Matrix<M, P, Field> result;
  for (size_t i = 0; i < M; ++i) {
    for (size_t j = 0; j < P; ++j) {
      for (size_t k = 0; k < N; ++k) {
        result.data_[i][j] += matrix1.data_[i][k] * matrix2.data_[k][j];
      }
    }
  }
  return result;
}

template <size_t M, size_t N, typename Field>
Matrix<M, M, Field>& Matrix<M, N, Field>::operator*=(const Matrix<N, N, Field>& matrix) {
  static_assert(M == N, "Matrix sizes for multiplication do not match");
  *this = (*this) * matrix;
  return *this;
}

template <size_t M, size_t N, typename Field = Rational>
bool operator==(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
  return (matrix1.data_ == matrix2.data_);
}

template <size_t M, size_t N, typename Field = Rational>
bool operator!=(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
  return !(matrix1 == matrix2);
}

template <size_t M, typename Field = Rational>
using SquareMatrix = Matrix<M, M, Field>;

template <size_t M, size_t N, typename Field>
std::ostream& operator<<(std::ostream& os, const Matrix<M, N, Field>& matrix) {
  os << "{";
  for (size_t i = 0; i < M; ++i) {
    os << "{";
    for (size_t j = 0; j < N; ++j) {
      os << matrix[i][j];
      if (j != N - 1) {
        os << ", ";
      }
    }
    os << '}';
    if (i != M - 1) {
      os << ", ";
    }
  }
  os << '}' << std::endl;
  return os;
}

template <size_t M, size_t N, typename Field>
std::istream& operator>>(std::istream& is, Matrix<M, N, Field>& matrix) {
  for (size_t i = 0; i < M; ++i) {
    for (size_t j = 0; j < N; ++j) {
      is >> matrix[i][j];
    }
  }
  return is;
}

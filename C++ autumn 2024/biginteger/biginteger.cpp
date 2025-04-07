#include <iostream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "biginteger.h"

// ТЕСТЫ 2023-2024 (потом были усложнены)

using namespace std;

template<typename T>
void testBigInteger() {

  T b, k;
  b = 0, k = 1234567;
  std::ostringstream oss;
  oss << b << ' ' << k;
  if (oss.str() != "0 1234567") {
    throw std::runtime_error("Assignment from int, or stream output failed.");
  }

  T a = b;
  a = -a;
  std::string testString = a.toString() + " " + (-k).toString();
  if (testString != "0 -1234567") {
    throw std::runtime_error("Unary minus or toString method failed.");
  }

  a = 999, b = 1000;
  a = a += a;
  testString = a.toString();
  if (testString != "1998") {
    throw std::runtime_error("Operator = or += failed.");
  }

  ++a %= a /= a *= a -= b++;
  std::ostringstream oss2;
  oss2 << 5 + a << ' ' << 1 - b; // 5 -1000
  if (oss2.str() != "5 -1000") {
    throw std::runtime_error("Arithmetic operations failed.");
  }

  std::ostringstream oss3;
  oss3 << (a = (bool(a) ? -1 : -2));
  BigInteger m = -2;
  if (oss3.str() != "-2") {
    throw std::runtime_error("Tests failed.");
  }

  std::istringstream iss("26 5");
  iss >> a >> b;
  std::ostringstream oss4;
  oss4 << b << ' ' << a << ' ';
  if (oss4.str() != "5 26 ") {
    throw std::runtime_error("Stream input or output failed.");
  }

  oss4 << a + b << ' ' << a - b << ' ' << a * b << ' ' << a / b << ' ' << a % b << '\n';
  oss4 << b + a << ' ' << b - a << ' ' << b * a << ' ' << b / a << ' ' << b % a;
  if (oss4.str() != "5 26 31 21 130 5 1\n31 -21 130 0 5") {
    throw std::runtime_error("Arithmetic operations failed.");
  }

  std::vector<T> v;
  for (size_t i = 0; i < 1000; ++i) {
    v.push_back(1000 - i);
  }
  std::sort(v.begin(), v.end());

  std::ostringstream oss5;
  oss5 << v[0] << ' ' << v[500] << ' ' << v[999] << ' ';

  oss5 << (a != b) << ' ' << (a < b) << ' ' << (a > b) << ' ';
  oss5 << (a <= b) << ' ' << (a >= b);
  if (oss5.str() != "1 501 1000 1 0 1 0 1") {
    throw std::runtime_error("Relation operations failed.");
  }

  std::istringstream iss2("1000000000000000000000000000000000 -1000000");
  iss2 >> a;
  iss2 >> b;
  std::ostringstream oss6;
  oss6 << b << a;

  if (oss6.str() != "-10000001000000000000000000000000000000000") {
    throw std::runtime_error("Stream input or output failed.");
  }

  std::istringstream iss3("453234523460009834520987234598234502345987029345436345634563 "
                          "234523452034623049872345234520983475325345234232578");
  T c, d;
  iss3 >> c >> d;

  std::istringstream iss4("-23534576554950000000000000009999990000999900000 "
                          "8888888888884444444444433333333333332222222222222111112222777777777");
  T e, f;
  iss4 >> e >> f;
  std::ostringstream oss7;
  oss7 << a + b << ' ' << c + d << ' ' << e + f;
  if (oss7.str() != "999999999999999999999999999000000 "
                    "453234523694533286555610284470579736866970504670781579867141 "
                    "8888888888884444444420898756778383332222222222212111122221777877777") {
    throw std::runtime_error("Operator + failed.");
  }
  std::ostringstream oss8;
  oss8 << a - b << ' ' << c - d << ' ' << e - f;
  if (oss8.str() != "1000000000000000000000000001000000 "
                    "453234523225486382486364184725889267825003554020091111401985 "
                    "-8888888888884444444467967909888283332222222222232111102223777677777") {
    throw std::runtime_error("Operator - failed.");
  }
  std::ostringstream oss9;
  oss9 << a * b << ' ' << c * d << ' ' << e * f;
  if (oss9.str() != "-1000000000000000000000000000000000000000 "
                    "106294125023108851855435239407742287054686671354449187194200406809777590661604024572718537860109672117737393414 "
                    "-209196236043895401881977738593593744982694026094492829962212043149123345328234038901116544451103777729999222300000") {
    throw std::runtime_error("Operator * failed.");
  }

  std::ostringstream oss10;
  oss10 << a / b << ' ';
  oss10 << c / d << ' ';
  oss10 << e / f;
  if (oss10.str() != "-1000000000000000000000000000 1932576548 0") {
    throw std::runtime_error("Operator / failed. 10");
  }

  std::ostringstream oss11;
  oss11 << a % b << ' ' << c % d << ' ' << e % f;
  BigInteger n = (a % b);
  BigInteger n1 (0);
  bool t = n1 == n;
  if (oss11.str() != "0 101894444317458440603421824036688159663989325253819 "
                     "-23534576554950000000000000009999990000999900000") {
    std::cerr << "0";
  }
}

template <typename R, typename BI> void testRational() {
  R r;
  r = 5;
  r += 3;
  r *= 7; // r==56
  BI b = 15;
  (r /= 8) -= b; //r = 7-15=-8
  if (-r != 8)
    throw std::runtime_error("Test 1 failed.");

  R s, t;
  s = R(85)/37, t = R(29)/BI(-163);
  s += t;
  t = 1;
  for (int i = 0; i < 15; ++i)
    t *= s;
  if ((1/t).toString() != "507972178875842800075597772950831264898404875587494819951"
                          "/39717526884730183825748150063721595142668632339785349901549568")
    throw std::runtime_error("Test 2 failed.");
  s = 4*3*7*13*19*41*43*11; // 2^2×3×7×13×19×41×43×11
  t = -17*13*23*79;
  s *= s*s, t *= t*t;
  R q = s/t;
  if (q.toString() != "-29650611427828166204352/29472131485369")
    throw std::runtime_error("Test 3 failed.");

  if (q/1000000000 >= 1)
  throw std::runtime_error("Test 4 failed.");
  if (0/q != 0)
    throw std::runtime_error("Test 5 failed.");
  q *= t/s;
  if (q != 1 || q.toString() != "1")
    throw std::runtime_error("Test 6 failed.");

  s = 4*3*7*13*19*41*43*11;
  t = s - 25; // t=402365939
  ((s = 1000000007) *= 1000000009) *= 2147483647;
  if ((s/t).asDecimal(10) != "5337140829307966068.3989202202")
    throw std::runtime_error("Test 7 failed.");
  t = -t;
  std::string ssss;
  ssss = (t/s).asDecimal(25);
  if ((t/s).asDecimal(25) != "-0.0000000000000000001873662")
    throw std::runtime_error("Test 8 failed.");
}

signed main() {
   testBigInteger<BigInteger>();
   testRational<Rational, BigInteger>();
  return 0;
}
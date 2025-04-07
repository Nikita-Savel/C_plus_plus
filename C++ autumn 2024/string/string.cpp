#include "string.h"

int main() {
  String s("abcde");
  String s2("e");
  std::cout << (s = (s + s2)).length() << std::endl;
  s += s;
  std::cout << s << std::endl;
  std::cout << s.rfind("e");
}
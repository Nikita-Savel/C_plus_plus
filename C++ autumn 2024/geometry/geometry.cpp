#include "geometry.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

void testPolygonSimilarity() {
  Point p1(0, 0);
  Point p2(1, 0);
  Point p3(0, 1);
  Triangle triangle(p1, p2, p3);
  Circle circle = triangle.circumscribedCircle();

  std::cout << circle.focus1.x << circle.focus1.y << std::endl;
}

int main() {
    testPolygonSimilarity();
}
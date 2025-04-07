#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

const double kModule = 1e-9;

struct Vector;

class Line;

struct Point {
  double x;
  double y;

  Point() : x(0.0), y(0.0) {}
  Point(double x, double y) : x(x), y(y) {}
  Point(const Point& point) : x(point.x), y(point.y) {}

  Point& operator=(const Point& point) {
    x = point.x;
    y = point.y;
    return *this;
  }

  bool operator==(const Point& point) const {
    return std::abs(x - point.x) < kModule && std::abs(y - point.y) < kModule;
  }

  bool operator!=(const Point& point) const {
    return !(*this == point);
  }

  Point operator-=(const Point& point) const;

  bool operator>(const Point& point) const {
    return (x - point.x > kModule) || (std::abs(x - point.x) < kModule && (y - point.y > kModule));
  }

  Point operator+(const Vector& vector) const;

  void rotate(const Point& center, double angle) {
	*this -= center;
    Point rotated = *this;
    angle = (angle * M_PI) / 180.0;

    (*this).x = rotated.x * cos(angle) - rotated.y * sin(angle) + center.x;
    (*this).y = rotated.x * sin(angle) + rotated.y * cos(angle) + center.y;
  }

  void reflect(const Point& center);

  void reflect(const Line& axis);

  void scale(const Point& center, double coefficient);
};

Point operator+(const Point& point1, const Point& point2) {
    return Point(point1.x + point2.x, point1.y + point2.y);
}

Point operator-(const Point& point1, const Point& point2) {
    return Point(point1.x - point2.x, point1.y - point2.y);
}

Point operator*(double number, const Point& point) {
    return Point(point.x * number, point.y * number);
}

Point operator/(const Point& point, double number) {
    return Point(point.x / number, point.y / number);
}

Point Point::operator-=(const Point& point) const {
  return (*this - point);
}

void Point::reflect(const Point& center) {
    *this = 2 * center - *this;
}

void Point::scale(const Point& center, double coefficient) {
    *this = coefficient * (*this - center) + center;
}

struct Vector {
    double x;
    double y;

    Vector(const Point& a, const Point& b) : x(b.x - a.x), y(b.y - a.y) {}

    Vector(double x, double y) : x(x), y(y) {}

    Vector operator*=(double num) const;

    Vector operator/=(double num) const;

    double length() const {
      return std::sqrt(x * x + y * y);
    }
};

Vector operator+(const Vector& v1, const Vector& v2) {
  return Vector(v1.x + v2.x, v1.y + v2.y);
}

Vector operator/(const Vector& v1, double num) {
  return Vector(v1.x / num, v1.y / num);
}

Vector operator*(const Vector& v1, double num) {
  return Vector(v1.x * num, v1.y * num);
}

Vector Vector::operator*=(double num) const {
  return *this * num;
}

Vector Vector::operator/=(double num) const {
  return *this / num;
}

Point Point::operator+(const Vector& vector) const {
  return Point(x + vector.x, y + vector.y);
}

class Line {
private:
  double a;
  double b;
  double c;

public:
  Line(const Point& point1, const Point& point2) {
	double X = point2.x - point1.x;
    double Y = point2.y - point1.y;
    a = Y;
    b = -X;
    c = point1.y * X - point1.x * Y;
  }

  Line(const Point& point1, double slope) : a(slope) {
    b = -1;
    c = point1.y - slope * point1.x;
  }

  Line(double slope, double intercept) : a(slope), b(-1.0), c(intercept) {}

  Line(double a, double b, double c) : a(a), b(b), c(c) {}

  bool operator==(const Line& line) const {
    double koefficient = a / line.a;
    return std::abs(b / line.b - koefficient) < kModule && std::abs(c / line.c - koefficient) < kModule;
  }

  bool operator!=(const Line& line) const {
    return !(*this == line);
  }

  double getA() const {
    return a;
  }

  double getB() const {
    return b;
  }

  double getC() const {
    return c;
  }
};

Point SLE(const Line& line1, const Line& line2) {
	double d = line1.getA() * line2.getB() - line2.getA() * line1.getB();
    double dx = -1 * line1.getC() * line2.getB() + line2.getC() * line1.getB();
    double dy = -1 * line1.getA() * line2.getC() + line2.getA() * line1.getC();

    Point center(dx / d, dy / d);
    return center;
}

void Point::reflect(const Line& axis) {
  if (std::abs(axis.getA() * (*this).x + axis.getB() * (*this).y + axis.getC()) < kModule) {
    return;
  }
  Vector normal(axis.getB(), -axis.getA());
  double C = -normal.x * (*this).x - normal.y * (*this).y;
  Line line(normal.x, normal.y, C);
  Point intersect = SLE(line, axis);
  reflect(intersect);
}

class Shape {

public:
  virtual double perimeter() const = 0;
  virtual double area() const = 0;
  virtual bool isCongruentTo(const Shape& another) const = 0;
  virtual bool isSimilarTo(const Shape& another) const = 0;
  virtual bool containsPoint(const Point& point) const = 0;

  virtual void rotate(const Point& center, double angle) = 0;
  virtual void reflect(const Point& center) = 0;
  virtual void reflect(const Line& axis) = 0;
  virtual void scale(const Point& center, double coefficient) = 0;

  virtual ~Shape() = default;
};

class Polygon : public Shape {
protected:
  std::vector<Point> vertices;

  Polygon(int size) : vertices(size) {}

public:
	Polygon(const std::vector<Point>& vertices) : vertices(vertices) {}

    template <typename... Args>
	Polygon(Args... args) {
  		vertices = {args...};
	}

    size_t verticesCount() const {
        return vertices.size();
    }

    std::vector<Point> getVertices() const {
        return vertices;
    }

    int func(Vector vec1, Vector vec2) {
        if (vec1.x * vec2.y - vec1.y * vec2.x > 0) {
            return 1;
        } else if (vec1.x * vec2.y - vec1.y * vec2.x < 0) {
            return -1;
        }
        return 0;
    }

    bool isConvex() {
        Point memory1;
        Point memory2;
        Point start1;
        Point start2;
        Point end1;
        Point end2;
        int n = vertices.size();
		int flag = 0;
        for (int i = 0; i < n; i++) {
            Point point(vertices[i].x, vertices[i].y);
            if (i == 0) {
                start1 = point;
            }
            if (i == 1) {
                start2 = point;
            }
            if (i == 2) {
                flag = func(Vector(memory2, memory1), Vector(memory1, point));
            }
            if (i > 2) {
                if (flag == 0 && func(Vector(memory2, memory1), Vector(memory1, point)) != 0) {
                    flag = func(Vector(memory2, memory1), Vector(memory1, point));
                } else if (func(Vector(memory2, memory1), Vector(memory1, point)) != 0
                        && func(Vector(memory2, memory1), Vector(memory1, point)) != flag) {
                    return false;
                }
            }
            if (i == n - 2) {
                end2 = point;
            }
            if (i == n - 1) {
                end1 = point;
            }
            memory2 = memory1;
            memory1 = point;
        }
        if (flag == 0 && func(Vector(end2, end1), Vector(end1, start1)) != 0) {
            flag = func(Vector(end2, end1), Vector(end1, start1));
        } else if (func(Vector(end2, end1), Vector(end1, start1)) != 0
                && func(Vector(end2, end1), Vector(end1, start1)) != flag) {
            return false;
        }
        if (flag == 0 && func(Vector(end1, start1), Vector(start1, start2)) != 0) {
            flag = func(Vector(end1, start1), Vector(start1, start2));
        } else if (func(Vector(end1, start1), Vector(start1, start2)) != 0
                && func(Vector(end1, start1), Vector(start1, start2)) != flag) {
            return false;
        }
        return true;
    }

    double perimeter() const override {
        double current_perimeter = 0.0;
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t next_point = (i + 1) % vertices.size();
            double dx = vertices[next_point].x - vertices[i].x;
            double dy = vertices[next_point].y - vertices[i].y;
            current_perimeter += std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));
        }
        return current_perimeter;
    }

    double area() const override {
		double result = 0.0;
    	for (size_t i = 0; i < vertices.size(); ++i) {
      		const Point& p1 = vertices[i];
     		const Point& p2 = vertices[(i + 1) % vertices.size()];
      		result += (p1.x * p2.y - p2.x * p1.y);
    	}
    	return 0.5 * fabs(result);
    }

    double distance(const Point& p1, const Point& p2) const {
        return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
    }

    std::vector<double> getSideLengths() const {
        std::vector<double> lengths;
        for (size_t i = 0; i < vertices.size(); ++i) {
            int next = (i + 1) % vertices.size();
            lengths.push_back(distance(vertices[i], vertices[next]));
        }
        return lengths;
    }

    std::vector<double> getAngles() const {
        std::vector<double> angles;
        int n = vertices.size();
        for (int i = 0; i < n; ++i) {
            const Point p1 = vertices[i];
            const Point p2 = vertices[(i + 1) % n];
            const Point p3 = vertices[(i + 2) % n];

            double a = distance(p1, p2);
            double b = distance(p2, p3);
            double c = distance(p3, p1);
            double angle = std::acos((a * a + b * b - c * c) / (2 * a * b));
            angles.push_back(angle);
        }
        return angles;
    }

    bool isSimilarTo(const Shape& another) const override {
      const Polygon* polygon_pointer = dynamic_cast<const Polygon*>(&another);

      if (polygon_pointer != nullptr) {
        if (vertices.size() != polygon_pointer->vertices.size()) {
      		return false;
    	}
		std::vector<double> angles1 = getAngles();
        std::vector<double> angles2 = polygon_pointer->getAngles();
        for (size_t i = 0; i < angles1.size(); i++) {
          for (size_t j = 0; j < angles2.size(); j++) {
            if (std::abs(angles1[(i + j) % angles1.size()] - angles2[j]) > kModule) {
              break;
            }
            if (j == angles2.size() - 1) {
              return true;
            }
          }
          std::reverse(angles2.begin() + 1, angles2.end());

          for (size_t j = 0; j < angles2.size(); j++) {
            if (std::abs(angles1[(i + j) % angles1.size()] - angles2[j]) > kModule) {
              break;
            }
            if (j == angles2.size() - 1) {
              return true;
            }
          }
        }
      }
      return false;
    }

    bool isCongruentTo(const Shape& another) const override {
      const Polygon* polygon_pointer = dynamic_cast<const Polygon*>(&another);

      if (polygon_pointer != nullptr) {
        if (isSimilarTo(another) && std::abs((*polygon_pointer).perimeter() - perimeter()) < kModule) {
          return true;
        }
      }
      return false;
    }

    bool isPointOnSegment(const Point& point, const Point& p1, const Point& p2) const {
        Vector P1P2(p1, p2);
        Vector P1M(p1, point);
        Vector MP1(point, p1);
        Vector MP2(point, p2);
        return P1P2.x * P1M.y - P1P2.y * P1M.x == 0 && MP1.x * MP2.x + MP1.y * MP2.y <= 0;
    }

    bool containsPoint(const Point& point) const override {
        int n = vertices.size();
		bool inside = false;
        for (int i = 0; i < n; i++) {
            const Point pi = vertices[(i + 1) % n];
            const Point pj = vertices[i];
            if ((pi.y > point.y) != (pj.y > point.y)
                    && (point.x < (pj.x - pi.x) * (point.y - pi.y) / (pj.y - pi.y) + pi.x)) {
                inside = !inside;
            }
            if (isPointOnSegment(point, pi, pj)) {
                return true;
            }
        }
        return inside;
    }

    void rotate(const Point& center, double angle) override {
      for (Point& point : vertices) {
    	point.rotate(center, angle);
 	  }
    }

    void reflect(const Point& center) override {
      for (Point& point : vertices) {
    	point.reflect(center);
  	  }
    }

    void reflect(const Line& axis) override {
      for (Point& point : vertices) {
    	point.reflect(axis);
 	  }
    }

    void scale(const Point& center, double coefficient) override {
      for (Point& point : vertices) {
    	point.scale(center, coefficient);
  	  }
    }

    bool isEquals(const Shape& polygon2) const {
      const Polygon* polygon_ptr = dynamic_cast<const Polygon*>(&polygon2);
      if (polygon_ptr == nullptr
          || vertices.size() != polygon_ptr->getVertices().size()) {
        return false;
      }
      size_t n = vertices.size();
      int equals_index = -1;
      for (size_t i = 0; i < n; ++i) {
        if (vertices[0] == polygon_ptr->vertices[i]) {
          equals_index = static_cast<int> (i);
        }
      }
      if (equals_index == -1) {
        return false;
      }
      bool equals = true;
      for (size_t i = 0; i < n; ++i) {
        if (vertices[i] != polygon_ptr->vertices[(equals_index + i) % n]) {
          equals = false;
          break;
        }
      }

      if (equals) {
        return true;
      }
      equals = true;
      for (size_t i = 0; i < n; ++i) {
        if (vertices[i] != polygon_ptr->vertices[(equals_index - i + n) % n]) {
          equals = false;
          break;
        }
      }
      return equals;
    }
};

class Ellipse : public Shape {
public:
  Point focus1;
  Point focus2;
  double sum_distances;

public:
  Ellipse(const Point& f1, const Point& f2, double sum_distances)
        : focus1(f1), focus2(f2), sum_distances(sum_distances) {}

  double getSumDistances() const {
    return sum_distances;
  }

  Point getFocus1() const {
    return focus1;
  }

  Point getFocus2() const {
    return focus2;
  }

  std::pair<double, double> semiaxis() const {
    double a = sum_distances / 2; // Большая полуось
    double d = std::sqrt(std::pow(focus2.x - focus1.x, 2) + std::pow(focus2.y - focus1.y, 2));
    double b = std::sqrt(std::pow(a, 2) - std::pow(d / 2, 2)); // Малая полуось
    return {a, b};
  }

  std::pair<Point, Point> focuses() const {
    return {focus1, focus2};
  }

  double eccentricity() const {
    std::pair<double, double> semiaxises = semiaxis();
    double e = std::sqrt(std::pow(semiaxises.first, 2) - std::pow(semiaxises.second, 2)) / semiaxises.first;
    return e;
  }

  Point center() const {
    return Point((focus1.x + focus2.x) / 2, (focus1.y + focus2.y) / 2);
  }

  std::pair<Line, Line> directrices() const {
    double a = sum_distances / 2;
    double e = eccentricity();
    Point center_coords = center();

    Line directrix1(center_coords.x - a / e, center_coords.y);
    Line directrix2(center_coords.x + a / e, center_coords.y);

    return {directrix1, directrix2};
  }

  double perimeter() const override {
    std::pair<double, double> semiaxises = semiaxis();
    double a = semiaxises.first;
    double b = semiaxises.second;
    double perimeter_value = M_PI * (3 * (a + b) - std::sqrt((3 * a + b) * (a + 3 * b)));
    return perimeter_value;
  }

  double area() const override {
    double a = sum_distances / 2;
    double d = std::sqrt(std::pow(focus2.x - focus1.x, 2) + std::pow(focus2.y - focus1.y, 2));
    double b = std::sqrt(std::pow(a, 2) - std::pow(d / 2, 2));
    return M_PI * a * b;
  }

  bool isCongruentTo(const Shape& another) const override {
	const Ellipse* ellipse_pointer = dynamic_cast<const Ellipse*>(&another);
    if (ellipse_pointer != nullptr) {
      std::pair<double, double> current_semiaxis = semiaxis();
      std::pair<double, double> new_semiaxis = ellipse_pointer->semiaxis();

      return (std::abs(current_semiaxis.first - new_semiaxis.first) < kModule)
             && (std::abs(current_semiaxis.second - new_semiaxis.second) < kModule);
    }
    return false;
  }

  bool isSimilarTo(const Shape& another) const override {
	const Ellipse* ellipse_pointer = dynamic_cast<const Ellipse*>(&another);
    if (ellipse_pointer != nullptr) {
      std::pair<double, double> current_semiaxis = semiaxis();
      std::pair<double, double> new_semiaxis = ellipse_pointer->semiaxis();
	  double ratio1 = current_semiaxis.first / current_semiaxis.second;
      double ratio2 = new_semiaxis.first / new_semiaxis.second;

      return std::abs(ratio1 - ratio2) < kModule;
    }
    return false;
  }

  bool containsPoint(const Point& point) const override {
    std::pair<double, double> semiaxises = semiaxis();
	double d1 = std::sqrt(std::pow(point.x - focus1.x, 2) + std::pow(point.y - focus1.y, 2));
    double d2 = std::sqrt(std::pow(point.x - focus2.x, 2) + std::pow(point.y - focus2.y, 2));
    return (d1 + d2) <= 2 * semiaxises.first;
  }

  void rotate(const Point& center, double angle) override {
	focus1.rotate(center, angle);
  	focus2.rotate(center, angle);
  }

  void reflect(const Point& center) override {
	focus1.reflect(center);
  	focus2.reflect(center);
  }

  void reflect(const Line& axis) override {
	focus1.reflect(axis);
  	focus2.reflect(axis);
  }

  void scale(const Point& center, double coefficient) override {
    sum_distances *= coefficient;
    focus1.scale(center, coefficient);
    focus2.scale(center, coefficient);
  }

  bool isEquals(const Shape& ellipse2) const;
};

bool Ellipse::isEquals(const Shape& ellipse2) const {
  const Ellipse* ellipse_ptr = dynamic_cast<const Ellipse*>(&ellipse2);
  if (ellipse_ptr == nullptr) {
    return false;
  }
  double ellipse2_sum = ellipse_ptr->getSumDistances();
  Point ellipse2_focus_1 = ellipse_ptr->getFocus1();
  Point ellipse2_focus_2 = ellipse_ptr->getFocus2();
  return (std::abs(sum_distances - ellipse2_sum) < kModule)
    && ((focus1 == ellipse2_focus_1 && focus2 == ellipse2_focus_2)
    || (focus1 == ellipse2_focus_2 && focus2 == ellipse2_focus_1));
}

class Circle : public Ellipse {
public:
  Circle(Point center, double radius)
      : Ellipse(center, center, 2 * radius) {}

  double radius() const {
    return sum_distances / 2.0;
  }

  double perimeter() const override {
    return 2 * M_PI * radius();
  }

  double area() const override {
    return M_PI * std::pow(radius(), 2);
  }
};

class Rectangle : public Polygon {
public:
  Rectangle(const Point& point1, const Point& point2, double ratio)
       : Polygon(4) {
  if (ratio < 1) {
    ratio = 1 / ratio;
  }
  Point center((point1.x + point2.x) / 2, (point1.y + point2.y) / 2);
  double dx = point2.x - point1.x;
  double dy = point2.y - point1.y;
  double distance = std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));
  double angle = atan2(dy, dx);

  double width = distance / std::sqrt(1 + std::pow(ratio, 2));
  double height = ratio * width;

  if (width > height) {
    std::swap(width, height);
  }

  double semi_width = height / 2 * cos(angle);
  double semi_height = width / 2 * sin(angle);
  vertices[0] = Point(center.x + semi_width - semi_height, center.y + semi_width + semi_height);
  vertices[1] = Point(center.x - semi_width - semi_height, center.y + semi_width - semi_height);
  vertices[2] = Point(center.x - semi_width + semi_height, center.y - semi_width - semi_height);
  vertices[3] = Point(center.x + semi_width + semi_height, center.y - semi_width + semi_height);
}

  Point center() {
    Point& point1 = vertices[0];
    Point& point2 = vertices[2];
    return Point((point1.x + point2.x) / 2.0, (point1.y + point2.y) / 2.0);
  }

  std::pair<Line, Line> diagonals() {
    Line diagonal1 = Line(vertices[0], vertices[2]);
  	Line diagonal2 = Line(vertices[1], vertices[3]);
  	return {diagonal1, diagonal2};
  }
};

class Square : public Rectangle {
public:
  Square(const Point& p1, const Point& p2) : Rectangle(p1, p2, 1) {}

  Circle circumscribedCircle() {
    double len = std::sqrt(std::pow(vertices[2].x - vertices[0].x, 2) + std::pow(vertices[2].y - vertices[0].y, 2));
    double radius = len / 2;
    Point square_center = center();
    return Circle(square_center, radius);
  }

  Circle inscribedCircle() {
    double len = std::sqrt(std::pow(vertices[1].x - vertices[0].x, 2) + std::pow(vertices[1].y - vertices[0].y, 2));
    double radius = len / 2;
    Point square_center = center();
    return Circle(square_center, radius);
  }
};

class Triangle : public Polygon {
public:
  Triangle(Point first, Point second, Point third) : Polygon(first, second, third) {}

  Circle circumscribedCircle() {
    double a = distance(vertices[1], vertices[2]);
    double b = distance(vertices[0], vertices[2]);
    double c = distance(vertices[0], vertices[1]);
    double radius = (a * b * c) / (4.0 * area());
    double d = 2 * (vertices[0].x * (vertices[1].y - vertices[2].y) + vertices[1].x * (vertices[2].y - vertices[0].y)
                    + vertices[2].x * (vertices[0].y - vertices[1].y));
    double dx = 0;
    double dy = 0;
    for (int i = 0; i < 3; ++i) {
      dx += (std::pow(vertices[i].x, 2) + std::pow(vertices[i].y, 2)) * (vertices[(i + 1) % 3].y - vertices[(i + 2) % 3].y);
      dy += (std::pow(vertices[i].x, 2) + std::pow(vertices[i].y, 2)) * (vertices[(i + 2) % 3].x - vertices[(i + 1) % 3].x);
    }
    dx /= d;
    dy /= d;
    Point center(dx, dy);
    return Circle(center, radius);
  }

  Circle inscribedCircle() {
	double a = distance(vertices[1], vertices[2]);
    double b = distance(vertices[0], vertices[2]);
    double c = distance(vertices[0], vertices[1]);
    double perimeter = a + b + c;
    double radius = 2 * area() / perimeter;
    double x = (a * vertices[0].x + b * vertices[1].x + c * vertices[2].x) / perimeter;
    double y = (a * vertices[0].y + b * vertices[1].y + c * vertices[2].y) / perimeter;
    Point center(x, y);
    return Circle(center, radius);
  }

  Point centroid() {
	double x = (vertices[0].x + vertices[1].x + vertices[2].x) / 3.0;
    double y = (vertices[0].y + vertices[1].y + vertices[2].y) / 3.0;
    return Point(x, y);
  }

  Point intersection(const Line& first, const Line& second) {
    double module = first.getA() * second.getB() - second.getA() * first.getB();
    double x = -(second.getB() * first.getC() - first.getB() * second.getC()) / module;
    double y = -(first.getA() * second.getC() - second.getA() * first.getC()) / module;
    return Point(x, y);
  }

  Line perpendicular(const Line& line, const Point& point) {
    double a = line.getB();
    double b = -line.getA();
    double c = -1 * (line.getB() * point.x - line.getA() * point.y);
    Line perpendicular(a, b, c);
    return perpendicular;
  }

  Point orthocenter() {
    Line line1(vertices[0], vertices[1]);
    Line perpendicular1(perpendicular(line1, vertices[2]));
    Line line2(vertices[1], vertices[2]);
    Line perpendicular2(perpendicular(line2, vertices[0]));
    return intersection(perpendicular1, perpendicular2);
  }

  Line EulerLine() {
    Point current_centroid = centroid();
    Point current_orthocenter = orthocenter();
	return Line(current_centroid, current_orthocenter);
  }

  Circle ninePointsCircle() {
	Point point1 = Point((vertices[0].x + vertices[1].x) / 2, (vertices[0].y + vertices[1].y) / 2);
    Point point2 = Point((vertices[1].x + vertices[2].x) / 2, (vertices[1].y + vertices[2].y) / 2);
    Point point3 = Point((vertices[2].x + vertices[0].x) / 2, (vertices[2].y + vertices[0].y) / 2);
    Triangle current_triangle = Triangle(point1, point2, point3);
    Line line1(centroid(), orthocenter());
    Line line2(centroid(), current_triangle.circumscribedCircle().center());
    Line line3(current_triangle.circumscribedCircle().center(), orthocenter());
    return current_triangle.circumscribedCircle();
  }
};

bool operator==(const Shape& first, const Shape& second) {
  const Polygon* polygon_pointer = dynamic_cast<const Polygon*>(&first);
  if (polygon_pointer == nullptr) {
    const Ellipse* ellipse_pointer = dynamic_cast<const Ellipse*>(&first);
    if (ellipse_pointer != nullptr) {
      return ellipse_pointer->isEquals(second);
    }
    return false;
  }
  return polygon_pointer->isEquals(second);
}

#ifndef MONTE_CARLO_HPP
#define MONTE_CARLO_HPP

#include <ios>
#include <vector>

namespace rgb
{
  struct point_t
  {
    double x, y;
  };
  struct circle_t
  {
    point_t center;
    double r;
  };
  std::istream& operator>>(std::istream& in, point_t& point);
  std::istream& operator>>(std::istream& in, circle_t& circle);

  struct answer_t
  {
    size_t union_count, inter_count;
  };

  bool isInside(const point_t& point, const circle_t& circle);
  bool isInsideUnion(const point_t& point, const std::vector< circle_t >& circles);
  bool isInsideInter(const point_t& point, const std::vector< circle_t >& circles);

  double findMinX(const std::vector< circle_t >& circles);
  double findMaxX(const std::vector< circle_t >& circles);
  double findMinY(const std::vector< circle_t >& circles);
  double findMaxY(const std::vector< circle_t >& circles);

  answer_t calc(const std::vector< circle_t >& circles, size_t tests, size_t seed);
}

#endif

#include "monte-carlo.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <functional>
#include <random>

std::istream& rgb::operator>>(std::istream& in, point_t& point)
{
  std::istream::sentry s(in);
  if (!s)
  {
    return in;
  }
  return in >> point.x >> point.y;
}

std::istream& rgb::operator>>(std::istream& in, circle_t& circle)
{
  std::istream::sentry s(in);
  if (!s)
  {
    return in;
  }
  double dummy = 0;
  return in >> circle.r >> dummy >> circle.center;
}

bool rgb::isInside(const point_t& point, const circle_t& circle)
{
  double square_x = std::pow(std::abs(point.x - circle.center.x), 2);
  double square_y = std::pow(std::abs(point.y - circle.center.y), 2);
  return square_x + square_y <= std::pow(circle.r, 2);
}

bool rgb::isInsideUnion(const point_t& point, const std::vector< circle_t >& circles)
{
  auto func = std::bind(isInside, std::cref(point), std::placeholders::_1);
  return std::any_of(circles.begin(), circles.end(), func);
}

bool rgb::isInsideInter(const point_t& point, const std::vector< circle_t >& circles)
{
  auto func = std::bind(isInside, std::cref(point), std::placeholders::_1);
  return std::all_of(circles.begin(), circles.end(), func);
}

double rgb::findMinX(const std::vector< circle_t >& circles)
{
  if (circles.size() == 0)
  {
    return 0;
  }
  double min = circles[0].center.x - circles[0].r;
  for (size_t i = 1; i < circles.size(); i++)
  {
    min = std::min(min, circles[i].center.x - circles[i].r);
  }
  return min;
}

double rgb::findMaxX(const std::vector< circle_t >& circles)
{
  if (circles.size() == 0)
  {
    return 0;
  }
  double max = circles[0].center.x + circles[0].r;
  for (size_t i = 1; i < circles.size(); i++)
  {
    max = std::max(max, circles[i].center.x + circles[i].r);
  }
  return max;
}

double rgb::findMinY(const std::vector< circle_t >& circles)
{
  if (circles.size() == 0)
  {
    return 0;
  }
  double min = circles[0].center.y - circles[0].r;
  for (size_t i = 1; i < circles.size(); i++)
  {
    min = std::min(min, circles[i].center.y - circles[i].r);
  }
  return min;
}

double rgb::findMaxY(const std::vector< circle_t >& circles)
{
  if (circles.size() == 0)
  {
    return 0;
  }
  double max = circles[0].center.y + circles[0].r;
  for (size_t i = 1; i < circles.size(); i++)
  {
    max = std::max(max, circles[i].center.y + circles[i].r);
  }
  return max;
}

rgb::answer_t rgb::calc(const std::vector< circle_t >& circles, size_t tests, size_t seed)
{
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<> distrib_x(findMinX(circles), findMaxX(circles));
  std::uniform_real_distribution<> distrib_y(findMinY(circles), findMaxY(circles));
  std::uniform_real_distribution<> distrib_inter_x; // TODO: INTER NEED FOR FRAME
  std::uniform_real_distribution<> distrib_inter_y; // TODO: INTER NEED FOR FRAME

  size_t success_union = 0;
  size_t success_inter = 0;
  for (size_t i = 0; i < tests; ++i)
  {
    double x = distrib_x(engine);
    double y = distrib_y(engine);
    if (isInsideUnion({x, y}, circles))
    {
      ++success_union;
    }

    if (isInsideInter({x, y}, circles))
    {
      ++success_inter;
    }
  }
  return {success_union, success_inter};
}

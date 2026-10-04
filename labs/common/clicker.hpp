#ifndef CLICKER_HPP
#define CLICKER_HPP

#include <chrono>

namespace rgb
{
  class Clicker
  {
  public:
    Clicker();
    double millisec() const;
  private:
    std::chrono::time_point< std::chrono::system_clock > start_;
  };

  double monteCarloSquareThreads(size_t tries, size_t seed, double radius, size_t threads);
  void monteCarloSquareRef(size_t tries, size_t seed, double radius, size_t& res);
  size_t monteCarloSquare(size_t tries, size_t seed, double radius);
  double square(double radius);
}

#endif

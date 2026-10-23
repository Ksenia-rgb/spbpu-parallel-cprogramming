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
    std::chrono::time_point< std::chrono::high_resolution_clock > start_;
  };
}
#endif

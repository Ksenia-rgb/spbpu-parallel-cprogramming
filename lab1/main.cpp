#include <iostream>
#include <iomanip>
#include "clicker.hpp"

int main(int argc, char** argv)
{
  using namespace rgb;

  size_t tries = 0, seed = 0;
  if (argc < 2 || argc > 3)
  {
    std::cerr << "Error: arguments\n";
    return 1;
  }
  tries = std::strtoull(argv[1], nullptr, 10);
  if (tries == 0)
  {
    std::cerr << "Error: tries\n";
    return 1;
  }
  if (argc == 3)
  {
    seed = std::strtoull(argv[2], nullptr, 10);
  }

  double radius;
  size_t threads;
  while (std::cin >> radius >> threads)
  {
    std::cout << std::fixed << std::setprecision(3) <<
      monteCarloSquareThreads(tries, seed, radius, threads) <<
      ' ' << monteCarloSquareThreads(tries, seed, radius, 1) <<
      ' ' << square(radius);
    std::cout << '\n';
  }
}
#include <iostream>
#include <vector>
#include <limits>
#include <random>
#include <future>
#include "monte-carlo.hpp"

int main(int argc, char** argv)
{
  if (argc != 3 || argc != 4)
  {
    std::cerr << "Error: incorrect arguments count\n";
    return 1;
  }

  long long int threads = 0, tries = 0, seed_begin = 0;
  try
  {
    threads = std::strtoll(argv[1], nullptr, 10);
    tries = std::strtoll(argv[2], nullptr, 10);
    if (argc == 4)
    {
      seed_begin = std::strtoll(argv[3], nullptr, 10);
    }
    if (tries <= 0 || threads < 0 || seed_begin < 0)
    {
      std::cerr << "Error: incorrect arguments\n";
      return 1;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }

  std::vector< rgb::circle_t > circles;
  while (std::cin.eof())
  {
    rgb::circle_t circle;
    std::cin >> circle;
    if (std::cin.fail() && std::cin.eof())
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
    if (std::cin)
    {
      circles.push_back(circle);
    }
  }

  double min_x = rgb::findMinX(circles), max_x = rgb::findMaxX(circles);
  double min_y = rgb::findMinY(circles), max_y = rgb::findMaxY(circles);

  std::default_random_engine engine(seed_begin);
  std::uniform_real_distribution<> distrib;

  if (threads == 0)
  {
    rgb::answer_t res = rgb::calc(circles, tries, distrib(engine));
    double square_union = (max_x - min_x) * (max_y - min_y) * (res.union_count * 1.0 / tries);
    double square_inter = (res.union_count * 1.0 / tries); // TODO INTER NEED FRAME
    std::cout << square_union << ' ' << square_inter << '\n';
    return 0;
  }

  size_t max_threads = std::thread::hardware_concurrency();
  if (threads > max_threads)
  {
    threads = max_threads;
  }

  std::vector< std::future< rgb::answer_t > > futures;
  std::vector< size_t > results_union;
  std::vector< size_t > results_inter;
  futures.reserve(threads);
  results_union.reserve(threads + 1);
  results_inter.reserve(threads + 1);

  size_t per_th = tries / threads;
  for (size_t i = 0; i < threads; ++i)
  {
    size_t seed = distrib(engine) + i;
    futures.emplace_back(std::async(std::launch::async, rgb::calc, circles, per_th, seed));
  }
  for (size_t i = 0; i < threads; ++i)
  {
    rgb::answer_t res = futures[i].get();
    results_union.emplace_back(res.union_count);
    results_inter.emplace_back(res.inter_count);
  }
  rgb::answer_t res = rgb::calc(circles, tries - per_th * threads, distrib(engine) + threads);
  results_union.emplace_back(res.union_count);
  results_inter.emplace_back(res.inter_count);

  size_t success_points_union = std::accumulate(results_union.begin(), results_union.end(), 0);
  size_t success_points_inter = std::accumulate(results_inter.begin(), results_inter.end(), 0);
  double square_union = (max_x - min_x) * (max_y - min_y) * (success_points_union * 1.0 / tries);
  double square_inter = (success_points_inter * 1.0 / tries); // TODO INTER NEED FRAME

  std::cout << square_union << ' ' << square_inter << '\n';
  return 0;
}

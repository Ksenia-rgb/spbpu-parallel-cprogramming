#include <iostream>
#include <random>
#include <thread>
#include <future>

namespace
{
  double area(double r, size_t threads, size_t tests, size_t seed_begin);
  size_t calc(double r, size_t tests, size_t seed);
  bool isInside(double x, double y, double r);

  double area(double r, size_t threads, size_t tests, size_t seed_begin)
  {
    std::default_random_engine engine(seed_begin);
    std::uniform_real_distribution<> distrib;

    if (threads == 1)
    {
      return 4 * r * r * (calc(r, tests, distrib(engine)) * 1.0 / tests);
    }
  
    size_t max_threads = std::thread::hardware_concurrency();
    if (threads > max_threads)
    {
      threads = max_threads;
    }

    std::vector< std::future< size_t > > futures;
    std::vector< size_t > results;
    futures.reserve(threads);
    results.reserve(threads + 1);

    size_t per_th = tests / threads;
    for (size_t i = 0; i < threads; ++i)
    {
      size_t seed = distrib(engine) + i;
      futures.emplace_back(std::async(std::launch::async, calc, r, per_th, seed));
    }
    for (size_t i = 0; i < threads; ++i)
    {
      results.emplace_back(futures[i].get());
    }
    results.emplace_back(calc(r, tests - per_th * threads, distrib(engine) + threads));

    size_t success_points = std::accumulate(results.begin(), results.end(), 0.0);
    return 4 * r * r * (success_points * 1.0 / tests);
  }

  size_t calc(double r, size_t tests, size_t seed)
  {
    std::default_random_engine engine(seed);
    std::uniform_real_distribution<> distrib(-r, r);
    size_t success = 0;
    for (size_t i = 0; i < tests; ++i)
    {
      double x = distrib(engine);
      double y = distrib(engine);
      if (isInside(x, y, r))
      {
        ++success;
      }
    }
    return success;
  }

  bool isInside(double x, double y, double r)
  {
    return x * x + y * y <= r * r;
  }
}

int main()
{
  double radius = 0;
  size_t threads = 0, tests = 0, seed_begin = 0;
  if (!(std::cin >> radius >> threads >> tests >> seed_begin))
  {
    std::cerr << "Incorrect input: enter radius, threads count, tests count and seed begin value\n";
    return 1;
  }

  double res = area(radius, threads, tests, seed_begin);
  std::cout << "Area real: " << 3.14 * radius * radius << '\n';
  std::cout << "Area Monte-Carlo: " << res << '\n';
}



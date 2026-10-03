#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <chrono>
#include <thread>
#include <clicker.hpp>

using data_t = std::vector< unsigned long long >;
using value_t = data_t::value_type;

namespace
{
  value_t calc(size_t begin, size_t count, const data_t& array);

  value_t calc(size_t begin, size_t count, const data_t& array)
  {
    value_t sum = 0;
    for (size_t i = begin; i < begin + count; i++)
    {
      sum += array[i];
    }
    return sum;
  }
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Incorrect arguments\n";
    return 1;
  }
  std::ofstream fout(argv[1]);
  if (!fout.is_open())
  {
    std::cerr << "Incorrect file\n";
    return 1;
  }

  constexpr size_t size = 1'000'000'000;
  data_t values(size, 1);

  for (size_t i = 1; i <= 8; i++)
  {
    size_t thread_count = std::pow(2, i);
    size_t per_th = size / thread_count;

    std::vector< std::thread > threads;
    threads.reserve(thread_count);

    rgb::Clicker clicker;
    double init = clicker.millisec();

    for (size_t i = 0; i < thread_count; i++)
    {
      threads.emplace_back(std::thread(calc, i * per_th, per_th, std::cref(values)));
    }
    calc(thread_count * per_th, size % thread_count, std::cref(values));

    for (size_t i = 0; i < thread_count; i++)
    {
      threads[i].join();
    }

    size_t total = clicker.millisec();
    fout << "Threads: " << thread_count << " Time: " << total << '\n';
  }
}

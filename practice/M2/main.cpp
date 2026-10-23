#include <iostream>
#include <random>
#include <thread>
#include <future>

#include <windows.h>

namespace
{
  struct CalcConf
  {
    double r;
    size_t tests;
    size_t seed;
  };
  double area(double r, size_t threads, size_t tests, size_t seed_begin);
  unsigned __stdcall calc(void* data);
  bool isInside(double x, double y, double r);

  double area(double r, size_t threads_cnt, size_t tests, size_t seed_begin)
  {
    std::default_random_engine engine(seed_begin);
    std::uniform_real_distribution<> distrib;

    if (threads_cnt == 1)
    {
      CalcConf config{r, tests, static_cast< size_t >(distrib(engine))};
      return 4 * r * r * (calc(reinterpret_cast< void* >(&config)) * 1.0 / tests);
    }

    std::vector< CalcConf > configs;
    std::vector< HANDLE > threads;
    std::vector< size_t > results;
    configs.reserve(threads_cnt);
    threads.reserve(threads_cnt);
    results.reserve(threads_cnt + 1);

    size_t per_th = tests / threads_cnt;
    for (size_t i = 0; i < threads_cnt; ++i)
    {
      size_t seed = static_cast< size_t >(distrib(engine)) + i;
      CalcConf config{r, per_th, seed};
      configs.emplace_back(config);

      HANDLE thread = reinterpret_cast< HANDLE >(_beginthreadex(NULL, 0u, calc, reinterpret_cast< void* >(&configs.back()), 0, NULL));
      if (thread == NULL)
      {
        break;
      }
      threads.emplace_back(thread);
    }
    if (threads.size() != threads_cnt)
    {
      for (size_t i = 0; i < threads.size(); i++)
      {
        TerminateThread(threads[i], 1);
        CloseHandle(threads[i]);
      }
      throw std::logic_error("Error on creating threads");
    }
    DWORD res = WaitForMultipleObjects(threads_cnt, threads.data(), TRUE, INFINITE);
    if (res != WAIT_OBJECT_0)
    {
      for (size_t i = 0; i < threads.size(); i++)
      {
        DWORD res = WaitForSingleObject(threads[i], 0);
        if (res == WAIT_TIMEOUT)
        {
          TerminateThread(threads[i], 1);
        }
        CloseHandle(threads[i]);
      }
      throw std::logic_error("Error on waiting threads");
    }
    for (size_t i = 0; i < threads_cnt; i++)
    {
      DWORD res = 0;
      GetExitCodeThread(threads[i], &res);
      results.emplace_back(static_cast< size_t >(res));
    }
    CalcConf last{r, tests - per_th * threads_cnt, static_cast< size_t >(distrib(engine)) + threads_cnt};
    results.emplace_back(calc(reinterpret_cast< PVOID >(&last)));

    for (size_t i = 0; i < threads_cnt; i++)
    {
      CloseHandle(threads[i]);
    }

    size_t success_points = std::accumulate(results.begin(), results.end(), 0.0);
    return 4 * r * r * (success_points * 1.0 / tests);
  }

  unsigned calc(void* data)
  {
    CalcConf* config = reinterpret_cast< CalcConf* >(data);
    std::default_random_engine engine(config->seed);
    std::uniform_real_distribution<> distrib(-config->r, config->r);
    size_t success = 0;
    for (size_t i = 0; i < config->tests; ++i)
    {
      double x = distrib(engine);
      double y = distrib(engine);
      if (isInside(x, y, config->r))
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

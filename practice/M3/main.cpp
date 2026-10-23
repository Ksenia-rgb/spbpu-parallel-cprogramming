#include <iostream>
#include <random>
#include <string.h>

#include <pthread.h>

namespace
{
  struct CalcConf
  {
    const double r;
    const size_t tests;
    const size_t seed;
  };
  double area(double r, size_t threads, size_t tests, size_t seed_begin);
  void* calc(void* data);
  bool isInside(double x, double y, double r);

  double area(double r, size_t threads_cnt, size_t tests, size_t seed_begin)
  {
    std::default_random_engine engine(seed_begin);
    std::uniform_real_distribution<> distrib;

    if (threads_cnt == 1)
    {
      CalcConf config{r, tests, static_cast< size_t >(distrib(engine))};
      size_t res = reinterpret_cast< size_t >(calc(reinterpret_cast< void* >(&config)));
      return 4 * r * r * (res * 1.0 / tests);
    }

    std::vector< CalcConf > configs;
    std::vector< pthread_t > threads;
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

      pthread_t thread;
      int err = pthread_create(&thread, NULL, calc, reinterpret_cast< void* >(&configs.back()));
      if (err)
      {
        break;
      }
      threads.emplace_back(thread);
    }
    if (threads.size() != threads_cnt)
    {
      for (size_t i = 0; i < threads.size(); i++)
      {
        pthread_cancel(threads[i]);
      }
      std::string err_msg = "Error on creating threads: ";
      err_msg.append(strerror(errno));
      throw std::logic_error(err_msg);
    }
    for (size_t i = 0; i < threads_cnt; i++)
    {
      size_t code[1] = {};
      int err = pthread_join(threads[i], reinterpret_cast< void** >(&code));
      if (err)
      {
        for (size_t j = i + 1; j < threads_cnt; j++)
        {
          pthread_cancel(threads[i]);
        }
        std::string err_msg = "Error on creating threads: ";
        err_msg.append(strerror(err));
        throw std::logic_error(err_msg);
      }
      results.emplace_back(code[0]);
    }

    CalcConf last{r, tests - per_th * threads_cnt, static_cast< size_t >(distrib(engine)) + threads_cnt};
    size_t res = reinterpret_cast< size_t >(calc(reinterpret_cast< void* >(&last)));
    results.emplace_back(res);

    size_t success_points = std::accumulate(results.begin(), results.end(), 0.0);
    return 4 * r * r * (success_points * 1.0 / tests);
  }

  void* calc(void* data)
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
    return reinterpret_cast< void* >(success);
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

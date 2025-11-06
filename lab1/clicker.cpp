#include "clicker.hpp"
#include <random>
#include <iostream>
#include <thread>
#include <vector>

rgb::Clicker::Clicker():
	start_(std::chrono::high_resolution_clock::now())
{}

double rgb::Clicker::millisec() const
{
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;

	auto t = high_resolution_clock::now();
	return duration_cast< milliseconds >(t - start_).count();
}
double rgb::monteCarloSquareThreads(size_t tries, size_t seed, double radius, size_t threads)
{
	if (threads == 1)
	{
		Clicker cl{};
		double res = 4 * radius * radius * (monteCarloSquare(tries, seed, radius) * 1.0 / tries);
		std::cout << cl.millisec() << ' ';
		return res;
	}
	size_t max_threads = std::thread::hardware_concurrency();
	if (threads > max_threads)
	{
		threads = max_threads;
	}
	std::vector< std::thread > ths;
	ths.reserve(threads);
	std::vector< size_t > results(threads + 1, 0.0);

	Clicker cl{};
	size_t per_th = tries / threads;
	for (size_t i = 0; i < threads; ++i)
	{
		ths.emplace_back(monteCarloSquareRef, per_th, seed, radius, std::ref(results[i]));
	}
	monteCarloSquareRef(tries - per_th * threads, seed, radius, std::ref(results[threads]));
	for (auto&& th: ths)
	{
		th.join();
	}
	size_t success_points = std::accumulate(results.begin(), results.end(), 0.0);
	double res = 4 * radius * radius * (success_points * 1.0 / tries);
	std::cout << cl.millisec() << ' ';
	return res;
}
void rgb::monteCarloSquareRef(size_t tries, size_t seed, double radius, size_t& res)
{
	res += monteCarloSquare(tries, seed, radius);
}
size_t rgb::monteCarloSquare(size_t tries, size_t seed, double radius)
{
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<> narko_trib(-radius, radius);
	size_t success = 0;
	for (size_t i = 0; i < tries; ++i)
	{
		double x = narko_trib(generator);
		double y = narko_trib(generator);
		if (x * x + y * y <= radius * radius)
		{
			++success;
		}
	}
	return success;
}
double rgb::square(double radius)
{
	return 2 * acos(0.0) * radius * radius;
}

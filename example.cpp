#include "include/recursed_range.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

constexpr int kNumRuns = 10000;
constexpr int kNumElements = 500;

namespace detail
{

template<typename>
struct random_fill_helper;

} // namespace detail

template<typename T, typename... Arg>
void random_fill(T& x, Arg&&... arg)
{
  detail::random_fill_helper<T>::fill(x, std::forward<Arg>(arg)...);
}

namespace detail
{

std::vector<int> get_random_vector(int size)
{
  std::vector<int> vec;
  vec.resize(size);

  std::random_device rd { };
  std::mt19937 gen { rd() };
  std::uniform_int_distribution<> dist { 0, 15 };
  std::generate_n(begin(vec), size, [&](){ return dist(gen); });

  return vec;
}

template<typename T, typename Alloc>
struct random_fill_helper<std::vector<T, Alloc> >
{
  static void fill(std::vector<T, Alloc>& vec, int size)
  {
    vec = get_random_vector(size);
  }
};

template<typename T, std::size_t Sz>
struct random_fill_helper<std::array<T, Sz> >
{
  template<std::size_t... Idx>
  static void fill(std::array<T, Sz>& arr, int size, meta::index_sequence<Idx...>)
  {
    using expand = int[];
    (void)expand { 0, (random_fill(std::get<Idx>(arr), size), void(), 0)... };
  }

  static void fill(std::array<T, Sz>& arr, int size)
  {
    using indices = meta::make_index_sequence<Sz>;
    fill(arr, size, indices { });
  }
};

} // namespace detail

int main(int argc, char** argv)
{
  using Clock = std::chrono::high_resolution_clock;
  using Precision = std::chrono::microseconds;

  std::array<std::array<std::vector<int>, 2>, 3> data;
  random_fill(data, kNumElements);

  unsigned long long int native_timing = 0, native_sum = 0;
  unsigned long long int recurse_timing = 0, recurse_sum = 0;

  for(int i = 0; i < kNumRuns; ++i)
  {
    {
      Clock::time_point start = Clock::now();
      for(auto&& arr : data)
      {
        for(auto&& v : arr)
        {
          native_sum += std::accumulate(begin(v), end(v), 0);
        }
      }
      Clock::time_point end = Clock::now();

      native_timing += std::chrono::duration_cast<Precision>(end - start).count();
    }

    {
      auto rr = recurse_range(data);
      Clock::time_point start = Clock::now();
      recurse_sum += std::accumulate(begin(rr), end(rr), 0);
      Clock::time_point end = Clock::now();

      recurse_timing += std::chrono::duration_cast<Precision>(end - start).count();
    }
  }

  std::cout << "native  : " << std::setw(7) << (native_sum / kNumRuns) << " [" << ((double)native_timing / kNumRuns) << "us]" << std::endl;
  std::cout << "recurse : " << std::setw(7) << (recurse_sum / kNumRuns) << " [" << ((double)recurse_timing / kNumRuns) << "us]" << std::endl;

  std::cout << std::endl;
}

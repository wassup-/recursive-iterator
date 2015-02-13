#include "recursed_range.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <array>

int main(int argc, char** argv)
{
  {
    std::array<std::array<const std::array<int, 2>, 2>, 2> data {
      {
        {
          { { { 1, 2 } }, { { 3, 4 } } },
        },
        {
          { { { 5, 6 } }, { { 7, 8 } } },
        }
      },
    };

    auto r = recurse_range(data);
    std::cout << "size: " << r.size() << std::endl;
    std::copy(begin(r), end(r), std::ostream_iterator<int>(std::cout, " "));
  }

  std::cout << std::endl;

  {
    std::array<const std::array<std::vector<int>, 2>, 2> data {
      {
        {
          { { { 1, 3 } }, { { 2, 4 } } },
        },
        {
          { { { 5, 7 } }, { { 6, 8 } } },
        }
      },
    };

    auto r = recurse_range(data);
    std::cout << "size: " << r.size() << std::endl;
    std::copy(begin(r), end(r), std::ostream_iterator<int>(std::cout, " "));
  }

  std::cout << std::endl;
}

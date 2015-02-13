#include "recursed_range.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

template<typename Range>
void print_range_recursive(const Range& range)
{
  auto rr = recurse_range(range);

  using value_type = typename decltype(rr)::value_type;
  using output_iterator = std::ostream_iterator<value_type>;

  std::cout << "size: " << rr.size() << std::endl;
  std::copy(begin(rr),
            end(rr),
            output_iterator(std::cout, " "));
  std::cout << std::endl;
}

int main(int argc, char** argv)
{
  {
    // same kinds of subranges
    std::array<std::array<std::array<int, 2>, 2>, 2> data {
      {
        {
          { { { 1, 2 } }, { { 3, 4 } } },
        },
        {
          { { { 5, 6 } }, { { 7, 8 } } },
        }
      },
    };

    print_range_recursive(data);
  }

  std::cout << std::endl;

  {
    // different kinds of subranges
    std::array<std::array<std::vector<int>, 2>, 2> data {
      {
        {
          { { { 1, 3 } }, { { 2, 4 } } },
        },
        {
          { { { 5, 7 } }, { { 6, 8 } } },
        }
      },
    };

    print_range_recursive(data);
  }

  std::cout << std::endl;

  {
    // different kinds of subranges with const modifier
    std::array<std::vector<std::array<int, 2>>, 2> data {
      {
        {
          { { { 1, 2 } }, { { 3, 4 } } },
        },
        {
          { { { 5, 6 } }, { { 7, 8 } } },
        }
      },
    };

    print_range_recursive(data);
  }

  std::cout << std::endl;
}

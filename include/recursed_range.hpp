#ifndef RECURSED_RANGE_
#define RECURSED_RANGE_

#include "recursive_iterator.hpp"

#include <algorithm>

template<typename Range>
struct recursed_range
{
public:
  using iterator = recursive_iterator<Range>;
  using value_type = value_type_of<iterator>;
  using size_type = difference_type_of<iterator>;

public:
  recursed_range(Range& range)
  : range_(range)
  { }

  size_type depth() const
  {
    return iterator::triplets_size::value;
  }

  size_type size() const
  {
    using std::distance;
    return distance(begin(), end());
  }

  iterator begin() const
  {
    using std::begin;
    using std::end;

    auto first = begin(range_);
    auto last = end(range_);

    return { first, last, first };
  }

  iterator end() const
  {
    using std::begin;
    using std::end;

    auto first = begin(range_);
    auto last = end(range_);

    return { first, last, last };
  }

private:
  Range& range_;
};

template<typename Range>
static inline recursed_range<Range> recurse_range(Range& range)
{
  return { range };
}

#endif

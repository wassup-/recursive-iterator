#ifndef RECURSED_RANGE_
#define RECURSED_RANGE_

#include "recursive_iterator.hpp"

#include <algorithm>

template<typename Range>
struct recursed_range
{
public:
  using iterator = recursive_iterator<Range>;
  using size_type = difference_type_of<iterator>;

public:
  recursed_range(Range& range)
  : range_(range)
  { }

  size_type size() const
  {
    return std::distance(begin(), end());
  }

  iterator begin() const
  {
    using std::begin;
    using std::end;

    return { begin(range_), end(range_), begin(range_) };
  }

  iterator end() const
  {
    using std::begin;
    using std::end;

    return { begin(range_), end(range_), end(range_) };
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

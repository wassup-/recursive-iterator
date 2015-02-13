#ifndef RECURSIVE_ITERATOR_
#define RECURSIVE_ITERATOR_

#include "iterator_traits.hpp"

#include <iterator>
#include <tuple>

namespace detail
{

using True = std::true_type;
using False = std::false_type;

#define AUTO_RETURNS(x) -> decltype(x) { return x; }

template<typename Triplet>
inline auto get_begin_iter(Triplet&& triplet)
AUTO_RETURNS(std::get<0>(triplet))

template<typename Triplet>
inline auto get_cur_iter(Triplet&& triplet)
AUTO_RETURNS(std::get<1>(triplet))

template<typename Triplet>
inline auto get_end_iter(Triplet&& triplet)
AUTO_RETURNS(std::get<2>(triplet))

template<typename T>
using unqualified_t = typename std::remove_reference<typename std::remove_const<T>::type>::type;

template<typename Triplets>
inline auto first_triplet_in(Triplets&& triplets)
AUTO_RETURNS(std::get<0>(triplets))

template<typename Triplets>
inline auto last_triplet_in(Triplets&& triplets)
AUTO_RETURNS(std::get<(std::tuple_size<unqualified_t<Triplets>>::value - 1)>(triplets))

#undef AUTO_RETURNS

template<int Index, int Sz>
struct default_initializer
{
  template<typename Triplets, typename Bool>
  static bool initialize(Triplets& triplets, Bool initial)
  {
    using prev_index = std::integral_constant<int, (Index - 1)>;
    using cur_index = std::integral_constant<int, Index>;
    using next_index = std::integral_constant<int, (Index + 1)>;

    auto& prev_triplet = std::get<prev_index::value>(triplets);
    auto& cur_triplet = std::get<cur_index::value>(triplets);

    {
      auto& first = get_begin_iter(cur_triplet);
      auto& cur = get_cur_iter(cur_triplet);
      auto& last = get_end_iter(cur_triplet);

      auto& prev_iter = std::get<!Bool::value>(prev_triplet);
      first = cur = begin(*prev_iter);
      last = end(*prev_iter);
      if(first == last) return false;
    }

    using next_initializer = default_initializer<next_index::value, Sz>;
    return next_initializer::initialize(triplets, initial);
  }
};

template<int Sz>
struct default_initializer<0, Sz>
{
  enum { Index = 0 };

  template<typename Triplets, typename Bool>
  static bool initialize(Triplets& triplets, Bool initial)
  {
    using cur_index = std::integral_constant<int, Index>;
    using next_index = std::integral_constant<int, (Index + 1)>;

    auto& cur_triplet = std::get<cur_index::value>(triplets);

    {
      const auto& first = std::get<!Bool::value>(cur_triplet);
      const auto& last = get_end_iter(cur_triplet);
      if(first == last) return false;
    }

    using next_initializer = default_initializer<next_index::value, Sz>;
    return next_initializer::initialize(triplets, initial);
  }
};

template<int Sz>
struct default_initializer<Sz, Sz>
{
  template<typename Triplets, typename Bool>
  static bool initialize(Triplets& triplets, Bool initial)
  { return true; }
};

template<int Index>
struct default_incrementer
{
  template<typename Triplets>
  static bool increment(Triplets& triplets)
  {
    using prev_index = std::integral_constant<int, (Index - 1)>;
    using cur_index = std::integral_constant<int, Index>;
    using next_index = std::integral_constant<int, (Index + 1)>;

    auto& cur_triplet = std::get<cur_index::value>(triplets);

    auto& cur = get_cur_iter(cur_triplet);
    const auto& last = get_end_iter(cur_triplet);

    if(cur != last)
    {
      cur = std::next(cur);
    }

    if(cur != last)
    {
      using next_initializer = default_initializer<next_index::value, std::tuple_size<Triplets>::value>;
      while(!next_initializer::initialize(triplets, False{})) increment(triplets);
      return true;
    } else {
      if(Index > 0) {
        using prev_incrementer = default_incrementer<prev_index::value + (Index == 0)>;
        return prev_incrementer::increment(triplets);
      } else {
        return false;
      }
    }
  }
};

template<typename Iterator>
using iterator_triplet = std::tuple<Iterator, Iterator, Iterator>;

} // namespace detail

template<typename Range,
         template<int, int> class Initializer = detail::default_initializer,
         template<int> class Incrementer = detail::default_incrementer>
struct recursive_iterator
: std::iterator<std::forward_iterator_tag, final_value_type_of<Range>>
{
public:
  using iterator = iterator_of<Range>;
  using chain = iterator_chain_of<Range>;
  using iterator_triplets = typename chain::template enclose<std::tuple, detail::iterator_triplet>;
  using recurse_depth = std::tuple_size<iterator_triplets>;
  using final_iterator = typename chain::last_type;

  using reference = reference_type_of<final_iterator>;

public:
  recursive_iterator(iterator first, iterator last, iterator cur)
  {
    auto& initial_triplet = detail::first_triplet_in(triplets_);
    initial_triplet = std::tie(first, cur, last);
    initialize();

    if(cur == last)
    {
      // we are the end iterator, so traverse the whole range until we reach the end
      initial_triplet = std::tie(first, first, last);
      while(increment()) { }
    }
  }

  final_iterator base() const
  {
    const auto& triplet = detail::last_triplet_in(triplets_);
    return detail::get_cur_iter(triplet);
  }

  reference operator*() const
  {
    return *(base());
  }

  recursive_iterator operator++(int)
  {
    recursive_iterator copy { *this };
    increment();
    return copy;
  }

  recursive_iterator& operator++()
  {
    increment();
    return *this;
  }

  bool operator==(const recursive_iterator& other) const
  {
    return (triplets_ == other.triplets_);
  }

  bool operator!=(const recursive_iterator& other) const
  { return !(*this == other); }

protected:
  inline void initialize()
  {
    using initializer = Initializer<0, recurse_depth::value>;
    initializer::initialize(triplets_, detail::True{});
  }

  inline bool increment()
  {
    using incrementer = Incrementer<(recurse_depth::value - 1)>;
    return incrementer::increment(triplets_);
  }

private:
  iterator_triplets triplets_;
};

#endif

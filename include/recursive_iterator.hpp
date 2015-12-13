#ifndef RECURSIVE_ITERATOR_
#define RECURSIVE_ITERATOR_

#include "iterator_traits.hpp"

#include <iomanip>
#include <iostream>
#include <iterator>
#include <tuple>

namespace detail
{

using True = std::true_type;
using False = std::false_type;

template<typename T>
using unqualified_t = meta::eval<
                        std::remove_const<
                          meta::eval<
                            std::remove_reference<T> > > >;

#define AUTO_RETURN(x) -> decltype(x) { return x; }

template<typename Triplet>
inline auto get_begin_iter(Triplet&& triplet)
AUTO_RETURN(std::get<0>(triplet));

template<typename Triplet>
inline auto get_cur_iter(Triplet&& triplet)
AUTO_RETURN(std::get<1>(triplet));

template<typename Triplet>
inline auto get_end_iter(Triplet&& triplet)
AUTO_RETURN(std::get<2>(triplet));

template<typename Triplets>
inline auto first_triplet_in(Triplets&& triplets)
AUTO_RETURN(std::get<0>(triplets));

template<typename Triplets>
inline auto last_triplet_in(Triplets&& triplets)
AUTO_RETURN(std::get<(std::tuple_size<unqualified_t<Triplets> >::value - 1)>(triplets));

#undef AUTO_RETURN

template<int Index, int Sz>
struct default_initializer
{
  template<typename Triplets, typename Bool>
  static bool initialize(Triplets& triplets, Bool initial)
  {
    using prev_index = std::integral_constant<int, (Index - 1)>;
    using cur_index = std::integral_constant<int, Index>;
    using next_index = std::integral_constant<int, (Index + 1)>;

    using std::get;
    auto& prev_triplet = get<prev_index::value>(triplets);
    auto& cur_triplet = get<cur_index::value>(triplets);

    auto& first = get_begin_iter(cur_triplet);
    auto& cur = get_cur_iter(cur_triplet);
    auto& last = get_end_iter(cur_triplet);

    auto& prev_range = (initial) ? get_begin_iter(prev_triplet) : get_cur_iter(prev_triplet);
    first = cur = begin(*prev_range);
    last = end(*prev_range);

    using next_initializer = default_initializer<next_index::value, Sz>;
    return (first != last) ? next_initializer::initialize(triplets, initial) : false;
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

    using std::get;
    const auto& cur_triplet = get<cur_index::value>(triplets);

    const auto& first = (initial) ? get_begin_iter(cur_triplet) : get_cur_iter(cur_triplet);
    const auto& last = get_end_iter(cur_triplet);

    using next_initializer = default_initializer<next_index::value, Sz>;
    return (first != last) ? next_initializer::initialize(triplets, initial) : false;
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

    using std::get;
    auto& cur_triplet = get<cur_index::value>(triplets);

    auto& cur = get_cur_iter(cur_triplet);
    const auto& last = get_end_iter(cur_triplet);

    using std::advance;
    advance(cur, static_cast<int>(cur != last));

    if(cur == last)
    {
      using prev_incrementer = default_incrementer<prev_index::value + (Index == 0)>;
      return (Index > 0) ? prev_incrementer::increment(triplets) : false;
    }

    using next_initializer = default_initializer<next_index::value, std::tuple_size<Triplets>::value>;
    while(!next_initializer::initialize(triplets, False{})) if(!increment(triplets)) return false;
    return true;
  }
};

template<typename Iterator>
using iterator_triplet = std::tuple<Iterator, Iterator, Iterator>;

template<typename T, std::size_t... Indices>
void print_triplet(const iterator_triplet<T>& triplet, meta::index_sequence<Indices...>)
{
  using expand = int[];
  (void)expand { 0, ((std::cout << std::setw(16) << std::addressof(*(std::get<Indices>(triplet)))), void(), 0)... };
  std::cout << std::endl;
}

template<typename T>
void print_triplet(const iterator_triplet<T>& triplet)
{
  using indices = meta::make_index_sequence<3>;

  print_triplet(triplet, indices { });
}

template<typename Triplets, std::size_t... Indices>
void print_triplets(const Triplets& triplets, meta::index_sequence<Indices...>)
{
  using expand = int[];
  (void)expand { 0, (print_triplet(std::get<Indices>(triplets)), void(), 0)... };
}

template<typename Triplets>
void print_triplets(const Triplets& triplets)
{
  using indices = meta::make_index_sequence<std::tuple_size<Triplets>::value>;

  print_triplets(triplets, indices { });
}

} // namespace detail

template<typename Range,
         template<int, int> class Initializer = detail::default_initializer,
         template<int> class Incrementer = detail::default_incrementer>
struct recursive_iterator
: std::iterator<std::forward_iterator_tag, final_value_type_of<Range> >
{
public:
  using iterator = iterator_of<Range>;
  using iterator_chain = iterator_chain_of<Range>;
  using iterator_triplets = meta::apply_list<
                              meta::quote<std::tuple>,
                              meta::transform<
                                iterator_chain,
                                meta::quote<detail::iterator_triplet> > >;
  using triplets_size = std::tuple_size<iterator_triplets>;
  using final_iterator = meta::back<iterator_chain>;

  using reference = reference_type_of<final_iterator>;

public:
  recursive_iterator(iterator first, iterator last, iterator cur)
  {
    auto& initial_triplet = detail::first_triplet_in(triplets_);
    initial_triplet = std::tie(first, cur, last);
    initialize();

    if(cur == last)
    {
      // we are the end iterator, so traverse the whole range until we reach
      // the end to make our triplets represent the final state
      initial_triplet = std::tie(first, first, last);
      while(increment()) { }
    }
  }

  final_iterator base() const
  {
    const auto& last_triplet = detail::last_triplet_in(triplets_);
    return detail::get_cur_iter(last_triplet);
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
    auto&& my_last = detail::last_triplet_in(triplets_);
    auto&& their_last = detail::last_triplet_in(other.triplets_);
    return (my_last == their_last);
  }

  bool operator!=(const recursive_iterator& other) const
  { return !(*this == other); }

protected:
  inline void initialize()
  {
    using initializer = Initializer<0, triplets_size::value>;
    initializer::initialize(triplets_, detail::True{});
  }

  inline bool increment()
  {
    using incrementer = Incrementer<(triplets_size::value - 1)>;
    return incrementer::increment(triplets_);
  }

private:
  iterator_triplets triplets_;
};

#endif

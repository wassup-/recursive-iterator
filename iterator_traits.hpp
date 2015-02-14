#ifndef ITERATOR_TRAITS_
#define ITERATOR_TRAITS_

#include "meta.hpp"

#include <iterator>
#include <type_traits>

template<typename T>
meta::eval<std::add_lvalue_reference<T>>
ref_declval();

template<typename T>
meta::eval<std::add_lvalue_reference<meta::eval<std::add_const<T>>>>
cref_declval();

namespace detail
{

template<typename T>
struct iterator_of_impl
: meta::id<decltype(begin(ref_declval<T>()))>
{ };

} // namespace detail

template<typename T>
using iterator_of = detail::iterator_of_impl<T>;

template<typename Iterator>
using value_type_of = meta::id<typename std::iterator_traits<Iterator>::value_type>;

template<typename Iterator>
using reference_type_of = meta::id<typename std::iterator_traits<Iterator>::reference>;

template<typename Iterator>
using pointer_type_of = meta::id<typename std::iterator_traits<Iterator>::pointer>;

template<typename Iterator>
using difference_type_of = meta::id<typename std::iterator_traits<Iterator>::difference_type>;

template<typename T>
using iterator_of_t = meta::eval<iterator_of<T>>;

template<typename Iterator>
using value_type_of_t = meta::eval<value_type_of<Iterator>>;

template<typename Iterator>
using reference_type_of_t = meta::eval<reference_type_of<Iterator>>;

template<typename Iterator>
using pointer_type_of_t = meta::eval<pointer_type_of<Iterator>>;

template<typename Iterator>
using difference_type_of_t = meta::eval<difference_type_of<Iterator>>;

namespace detail
{

template<typename T>
struct is_iterable_impl
{
  using yes = char;
  using no = yes[2];

  template<typename U>
  static std::true_type test(decltype(begin(ref_declval<U>()))*,
                             decltype(end(ref_declval<U>()))*);
  template<typename U>
  static std::false_type test(...);

  using type = decltype(test<T>(nullptr, nullptr));
};

} // namespace detail

template<typename T>
using is_iterable = meta::eval<detail::is_iterable_impl<T>>;

namespace detail
{

template<typename Dst, typename Src>
struct with_const_of_impl : meta::id<Dst> { };
template<typename Dst, typename Src>
struct with_const_of_impl<Dst, const Src> : meta::id<const Dst> { };

} // namespace detail

template<typename Dst, typename Src>
using with_const_of = meta::eval<detail::with_const_of_impl<Dst, Src>>;

namespace detail
{

template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template<typename Range, typename Iterators, typename = std::false_type>
struct iterator_chain_of_impl : Iterators { };

template<typename Range, typename... Iters>
struct iterator_chain_of_impl<Range, meta::list<Iters...>, std::true_type>
{
  using value_type = typename Range::value_type;
  using prev_iterator = meta::back<meta::list<Iters...>>;
  using cv_type = remove_pointer_t<pointer_type_of_t<prev_iterator>>;
  using iterator = iterator_of_t<with_const_of<Range, cv_type>>;

  using type = meta::eval<iterator_chain_of_impl<value_type,
                                                 meta::list<Iters..., iterator>,
                                                 is_iterable<value_type>>>;
};

template<typename Range>
struct iterator_chain_of_impl<Range, meta::list<>, std::true_type>
{
  using value_type = typename Range::value_type;
  using iterator = iterator_of_t<Range>;

  using type = meta::eval<iterator_chain_of_impl<value_type,
                                                 meta::list<iterator>,
                                                 is_iterable<value_type>>>;
};

} // namespace detail

template<typename Range>
using iterator_chain_of = detail::iterator_chain_of_impl<Range, meta::list<>, is_iterable<Range>>;

template<typename Range>
using iterator_chain_of_t = meta::eval<iterator_chain_of<Range>>;

namespace detail
{

template<typename Range>
struct final_value_type_of_impl
: meta::id<value_type_of_t<meta::back<iterator_chain_of_t<Range>>>>
{ };

} // namespace detail

template<typename Range>
using final_value_type_of = detail::final_value_type_of_impl<Range>;

template<typename Range>
using final_value_type_of_t = meta::eval<final_value_type_of<Range>>;

#endif

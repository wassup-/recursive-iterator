#ifndef ITERATOR_TRAITS_
#define ITERATOR_TRAITS_

#include <iterator>
#include <type_traits>

template<typename T>
struct identity
{
  using type = T;
};

template<typename H, typename...>
struct first_type_of : identity<H> { };

template<typename...>
struct last_type_of;
template<typename H, typename... T>
struct last_type_of<H, T...> : last_type_of<T...> { };
template<typename T>
struct last_type_of<T> : identity<T> { };

template<typename... T>
struct type_sequence
{
  using type = type_sequence;

  template<typename U>
  using push_front = type_sequence<U, T...>;

  template<typename U>
  using push_back = type_sequence<T..., U>;

  template<template<typename...> class C>
  using as = C<T...>;

  template<template<typename...> class O, template<typename...> class I>
  using enclose = O<I<T>...>;

  using first_type = typename as<first_type_of>::type;
  using last_type = typename as<last_type_of>::type;
};

namespace detail
{

template<typename T>
struct iterator_of_impl
: identity<decltype(begin(std::declval<T>()))>
{ };

} // namespace detail

template<typename T>
using iterator_of = typename detail::iterator_of_impl<T>::type;

template<typename Iterator>
using value_type_of = typename std::iterator_traits<Iterator>::value_type;

template<typename Iterator>
using reference_type_of = typename std::iterator_traits<Iterator>::reference;

template<typename Iterator>
using difference_type_of = typename std::iterator_traits<Iterator>::difference_type;

namespace detail
{

template<typename T>
struct is_iterable_impl
{
  using yes = char;
  using no = yes[2];

  template<typename U>
  static std::true_type test(decltype(begin(std::declval<U>()))*,
                             decltype(end(std::declval<U>()))*);
  template<typename U>
  static std::false_type test(...);

  using type = decltype(test<T>(nullptr, nullptr));
};

} // namespace detail

template<typename T>
using is_iterable = typename detail::is_iterable_impl<T>::type;

namespace detail
{

template<typename Dst, typename Src>
struct with_const_of_impl : identity<Dst> { };
template<typename Dst, typename Src>
struct with_const_of_impl<Dst, const Src> : identity<const Dst> { };

} // namespace detail

template<typename Dst, typename Src>
using with_const_of = typename detail::with_const_of_impl<Dst, Src>::type;

namespace detail
{

template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template<typename Range, typename Iterators, typename = std::false_type>
struct iterator_chain_of_impl : Iterators { };

template<typename Range, typename... Iters>
struct iterator_chain_of_impl<Range, type_sequence<Iters...>, std::true_type>
{
  using value_type = typename Range::value_type;
  using prev_iterator = typename last_type_of<remove_pointer_t<Iters>...>::type;
  using iterator = iterator_of<with_const_of<Range, prev_iterator>>;

  using type = typename iterator_chain_of_impl<value_type,
                                               type_sequence<Iters..., iterator>,
                                               is_iterable<value_type>>::type;
};

template<typename Range>
struct iterator_chain_of_impl<Range, type_sequence<>, std::true_type>
{
  using value_type = typename Range::value_type;
  using iterator = iterator_of<Range>;

  using type = typename iterator_chain_of_impl<value_type,
                                               type_sequence<iterator>,
                                               is_iterable<value_type>>::type;
};

} // namespace detail

template<typename Range>
using iterator_chain_of = typename detail::iterator_chain_of_impl<Range, type_sequence<>, is_iterable<Range>>::type;

namespace detail
{

template<typename Range>
struct final_value_type
: identity<value_type_of<typename iterator_chain_of<Range>::last_type>>
{ };

} // namespace detail

template<typename Range>
using final_value_type_of = typename detail::final_value_type<Range>::type;

#endif

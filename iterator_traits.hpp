#ifndef ITERATOR_TRAITS_
#define ITERATOR_TRAITS_

#include <iterator>
#include <type_traits>

template<typename T>
struct identity
{
  using type = T;
};

template<typename T>
typename std::add_lvalue_reference<T>::type ref_declval();

template<typename T>
typename std::add_lvalue_reference<typename std::add_const<T>::type>::type cref_declval();

namespace detail
{

template<typename H, typename...>
struct first_type_of_impl : identity<H> { };

template<typename...>
struct last_type_of_impl;
template<typename H, typename... T>
struct last_type_of_impl<H, T...> : last_type_of_impl<T...> { };
template<typename T>
struct last_type_of_impl<T> : identity<T> { };

}

template<typename... T>
using first_type_of = typename detail::first_type_of_impl<T...>::type;

template<typename... T>
using last_type_of = typename detail::last_type_of_impl<T...>::type;

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

  using first_type = as<first_type_of>;
  using last_type = as<last_type_of>;
};

namespace detail
{

template<typename T>
struct iterator_of_impl
: identity<decltype(begin(ref_declval<T>()))>
{ };

} // namespace detail

template<typename T>
using iterator_of = typename detail::iterator_of_impl<T>::type;

template<typename Iterator>
using value_type_of = typename std::iterator_traits<Iterator>::value_type;

template<typename Iterator>
using reference_type_of = typename std::iterator_traits<Iterator>::reference;

template<typename Iterator>
using pointer_type_of = typename std::iterator_traits<Iterator>::pointer;

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
  static std::true_type test(decltype(begin(ref_declval<U>()))*,
                             decltype(end(ref_declval<U>()))*);
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
  using prev_iterator = last_type_of<Iters...>;
  using cv_type = remove_pointer_t<pointer_type_of<prev_iterator>>;
  using iterator = iterator_of<with_const_of<Range, cv_type>>;

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
struct final_value_type_of_impl
: identity<value_type_of<typename iterator_chain_of<Range>::last_type>>
{ };

} // namespace detail

template<typename Range>
using final_value_type_of = typename detail::final_value_type_of_impl<Range>::type;

#endif

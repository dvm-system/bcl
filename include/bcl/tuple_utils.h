//===- tuple_utils.h ---- Functions to Work with Tuples ---------*- C++ -*-===//
//
//                       Base Construction Library (BCL)
//
// Copyright 2021 Nikita Kataev
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//
//
// This file defines functions, which are not available in standard library,
// to work with tuples.
//
//===----------------------------------------------------------------------===//

#include <tuple>

#ifndef BCL_TUPLE_UTILS_H
#define BCL_TUPLE_UTILS_H

namespace bcl {
/// Evaluate to true if there are no elements into a tuple.
template<typename T> class tuple_empty :
  public std::conditional<(std::tuple_size<T>::value > 0),
    std::false_type, std::true_type>::type{};

namespace detail {
template<std::size_t Idx, typename Function, typename... Types>
void for_each(const std::tuple<Types...> &T, Function &&F, std::true_type) {}

template<std::size_t Idx, typename Function, typename... Types>
void for_each(std::tuple<Types...> &T, Function &&F, std::false_type) {
  using TupleT = std::tuple<Types...>;
  F.operator()(std::get<Idx>(T));
  using IsLast =
      typename std::conditional<(Idx + 1 < std::tuple_size<TupleT>::value),
                                std::false_type, std::true_type>::type;
  for_each<Idx + 1>(T, F, IsLast{});
}

template<std::size_t Idx, typename Function, typename... Types>
void for_each(const std::tuple<Types...> &T, Function &&F, std::false_type) {
  using TupleT = std::tuple<Types...>;
  F.operator()(std::get<Idx>(T));
  using IsLast =
      typename std::conditional<(Idx + 1 < std::tuple_size<TupleT>::value),
                                std::false_type, std::true_type>::type;
  for_each<Idx + 1>(T, F, IsLast{});
}
}

/// Call a specified function for each element in a tuple.
template<typename Function, typename... Types>
void for_each(std::tuple<Types...> &T, Function &&F) {
  detail::for_each<0>(T, std::forward<Function>(F),
                      tuple_empty<std::tuple<Types...>>{});
}

/// Call a specified function for each element in a tuple.
template<typename Function, typename... Types>
void for_each(const std::tuple<Types...> &T, Function &&F) {
  detail::for_each<0>(T, std::forward<Function>(F),
                      tuple_empty<std::tuple<Types...>>{});
}

namespace detail {
template <typename Function, typename... Types, std::size_t... Indexes>
decltype(auto) forward_as_args(Function &&F, std::tuple<Types...> &T,
                               std::index_sequence<Indexes...>) {
  return F(std::get<Indexes>(T)...);
}

template <typename Function, typename... Types, std::size_t... Indexes>
decltype(auto) forward_as_args(Function &&F, const std::tuple<Types...> &T,
                               std::index_sequence<Indexes...>) {
  return F(std::get<Indexes>(T)...);
}
}

/// Forward elements of T to F.
template<typename Function, typename... Types>
decltype(auto) forward_as_args(std::tuple<Types...> &T, Function &&F) {
  return detail::forward_as_args(std::forward<Function>(F), T,
                                 std::index_sequence_for<Types...>());
}

/// Forward elements of T to F.
template<typename Function, typename... Types>
decltype(auto) forward_as_args(const std::tuple<Types...> &T, Function &&F) {
  return detail::forward_as_args(std::forward<Function>(F), T,
                                 std::index_sequence_for<Types...>());
}
}
#endif//BCL_TUPLE_UTILS_H

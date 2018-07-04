//===- tq_test.h -- Transparent Queue Correctness Test ---------*- C -*-===//
//
//                       Base Construction Library (BCL)
//
// Copyright 2018 Nikita Kataev
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
// This file implements utils to simplify testing of bcl::transparent_queue.
//
//===----------------------------------------------------------------------===//

#include <bcl/transparent_queue.h>
#include <iostream>
#include <utility>
#include <string>
#include <type_traits>

namespace bcl {
namespace test {
/// Extracts items from a specified queue TQ and prints them to O. Prints name
/// Id of the queue before its items.
template<class T, class OsT>
void pop_print(const std::string &Id, TransparentQueue<T> &TQ, OsT &O) {
  auto Tmp(TQ);
  O << "Queue " << Id << ": ";
  while (!Tmp.empty()) {
    auto *Item = Tmp.pop();
    O << Item << "(" << *Item << ") ";
  }
  O << std::endl;
}

/// Compares queues, prints and returns results of comparison, prints queue.
template<class T, class OsT>
std::pair<bool, bool> test_eq_lt(
    TransparentQueue<T> &Q1, TransparentQueue<T> &Q2, OsT &O) {
  bool EQ = (Q1 == Q2);
  bool LT = (Q1 < Q2);
  O << std::boolalpha << "Q1 == Q2: " << EQ <<" Q1 < Q2: " << LT << std::endl;
  pop_print("Q1", Q1, std::cout);
  pop_print("Q2", Q2, std::cout);
  return std::make_pair(EQ, LT);
}

/// Uses list of pointers to initalize queue.
template<class ListT>
auto tq_init(ListT &List) -> TransparentQueue<
    typename std::remove_pointer<typename ListT::value_type>::type> {
  using T = typename std::remove_pointer<typename ListT::value_type>::type;
  TransparentQueue<T> Q;
  for (T *Item : List)
    Q.push(Item);
  return Q;
}
}
}

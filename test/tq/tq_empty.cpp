//===- tq_empty.cpp -- Transparent Queue Correctness Test ---------*- C -*-===//
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
// This file implements comparison test for empty queues.
//
//===----------------------------------------------------------------------===//

#include "tq_test.h"
#include <bcl/bcl-config.h>

int main() {
  std::cout << "BCL version " << BCL_VERSION_STRING << std::endl;
  bcl::TransparentQueue<int> Q1, Q2;
  std::cout << "Empty queues: ";
  return std::make_pair(true, false) !=
    bcl::test::test_eq_lt(Q1, Q2, std::cout);
}


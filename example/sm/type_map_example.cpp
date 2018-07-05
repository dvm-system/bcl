//===- type_map_example.cpp - Static Type Map Usage Example -------*- C -*-===//
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
// This file presents an example of bcl::StaticTypeMap usage.
//
//===----------------------------------------------------------------------===//

#include <bcl/cell.h>
#include <iostream>
#include <string>
#include <typeinfo>

using namespace bcl;

struct PrintSizeFunctor {
  template<class Type> void operator()() {
    std::cout << "size of " << typeid(Type).name() << " is " << sizeof(Type) <<
      std::endl;
  }
};

struct PrintValueFunctor {
  template<class Type> void operator()(Type &V) {
    std::cout << "value of " << typeid(Type).name() << " is " << V << std::endl;
  }
};

int main() {
  // Defines static map where type of each value is treated as a key.
  StaticTypeMap<int, double, std::string> M;
  // Initializes each value in the map.
  M.value<int>() = 5;
  M.value<double>() = 6.3;
  M.value<std::string>() = "Hello!";
  // Prints size of each type in the map.
  std::cout << "Size of each type in the map:\n";
  PrintSizeFunctor PSF;
  M.for_each_key(PSF);
  // Prints each value in the map.
  std::cout << "Each value in the map:\n";
  PrintValueFunctor PVF;
  M.for_each(PVF);
  return 0;
}


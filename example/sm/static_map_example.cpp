//===- static_map_example.cpp -- Static Map Usage Example ---------*- C -*-===//
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
// This file presents an example of bcl::StaticMap usage.
//
//===----------------------------------------------------------------------===//

#include <bcl/cell.h>
#include <bcl/utility.h>
#include <iostream>
#include <string>

using namespace bcl;

struct Name {
  using ValueType = std::string;
  static inline const std::string & toString() {
    static const std::string Str("Name");
    return Str;
  }
};

struct Salary {
  using ValueType = double;
  static inline const std::string & toString() {
    static const std::string Str("Salary");
    return Str;
  }
};

/// This static map defines a table row which reflects the salary of workers.
using SalaryInfo = StaticMap<Name, Salary>;

/// This functor prints structure of a static map.
struct PrintStructureFunctor {
  template<class CellTy> void operator()() const {
    using CellKey = typename CellTy::CellKey;
    std::cout << CellKey::toString() << " is <value>\n";
  }
};

/// This functor prints worker's name and salary.
struct PrintSalaryFunctor {
  template<class CellTy> void operator()(const CellTy *C) const {
    using CellKey = typename CellTy::CellKey;
    std::cout << CellKey::toString() << " is ";
    std::cout << C->template value<CellKey>();
    std::cout << std::endl;
  }
};

int main() {
  std::cout << "Structure of salary information collection:" << std::endl;
  SalaryInfo::for_each_key(PrintStructureFunctor());
  SalaryInfo S;
  S.value<Name>() = "Smit";
  S.value<Salary>() = 300.50;
  std::cout << "Worker's salary is:" << std::endl;
  S.for_each(PrintSalaryFunctor());
  return 0;
}


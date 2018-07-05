//===- tq_perf.cpp ------- Transparent Queue Benchmark ------------*- C -*-===//
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
// This file implements performance benchmark for bcl::TransparentQueue.
//
//===----------------------------------------------------------------------===//

#include <bcl/bcl-config.h>
#include <bcl/transparent_queue.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <map>
#include<string>

using TimeT = std::chrono::duration<double>;

std::size_t initSmallSizes(unsigned Bound, unsigned SmallSize, std::size_t Size,
    unsigned *SmallSizes) {
  std::size_t BoundLessNum = 0;
  for (auto I = 0; I < Size; ++I) {
    SmallSizes[I] = std::rand() % SmallSize;
    if (SmallSizes[I] < Bound)
      ++BoundLessNum;
  }
  return BoundLessNum;
}

template<class QueueT>
QueueT pushTime(int **Init, std::size_t Size, TimeT &T) {
  auto S = std::chrono::high_resolution_clock::now();
  QueueT Q;
  for (int I = 0; I < Size; ++I)
    Q.push(Init[I]);
  auto E = std::chrono::high_resolution_clock::now();
  T += E - S;
  return Q;
}

template<class QueueT>
TimeT popTime(std::size_t Size, QueueT &Q) {
  auto S = std::chrono::high_resolution_clock::now();
  for (int I = 0; I < Size; ++I)
    Q.pop();
  auto E = std::chrono::high_resolution_clock::now();
  return E - S;
}

int main(int Argc, char **Argv) {
  std::string Help = "parameters: <size of data> [number of iterations]"
  " [maximum small size of data]\n";
  if (Argc < 2) {
    std::cerr << "error: too few arguments\n" << Help;
    return 1;
  } else if (Argc > 4) {
    std::cerr << "error: too many arguments\n" << Help;
    return 2;
  }
  std::size_t Size = std::atoll(Argv[1]);
  unsigned MaxIter = (Argc > 2) ? std::atoi(Argv[2]) : 10;
  unsigned SmallSize = (Argc > 3) ? std::atoi(Argv[3]) : 3;
  int **InitArray = new int * [Size];
  TimeT PushQ(0), PushTQ(0), PopQ(0), PopTQ(0);
  for (unsigned I = 0; I < MaxIter; ++I) {
    auto Q = pushTime<std::queue<int *>>(InitArray, Size, PushQ);
    PopQ += popTime(Size, Q);
    auto TQ = pushTime<bcl::TransparentQueue<int>>(InitArray, Size, PushTQ);
    PopTQ += popTime(Size, TQ);
  }
  unsigned *SmallSizes = new unsigned [Size];
  auto BoundLessNum = initSmallSizes(2, SmallSize, Size, SmallSizes);
  TimeT PushSmallQ(0), PushSmallTQ(0), PopSmallQ(0), PopSmallTQ(0);
  for (unsigned Iter = 0; Iter < MaxIter; ++Iter)
    for (std::size_t I = 0; I < Size; ++I) {
      auto Q = pushTime<std::queue<int *>>(
        InitArray, SmallSizes[I], PushSmallQ);
      PopSmallQ += popTime(SmallSizes[I], Q);
      auto TQ = pushTime<bcl::TransparentQueue<int>>(
        InitArray, SmallSizes[I], PushSmallTQ);
      PopSmallTQ += popTime(SmallSizes[I], TQ);
    }
  delete [] InitArray;
  delete [] SmallSizes;
  std::cout << "Results for " << __FILE__ << " benchmark" << std::endl;
  std::cout << "  date " << __DATE__ << std::endl;
  std::cout << "  compiler ";
#if defined __GNUC__
  std::cout << "GCC " << __GNUC__;
#elif defined __clang__
  std::cout << "Clang " << __clang__;
#elif defined _MSC_VER
  std::cout << "Microsoft " << _MSC_VER;
#else
  std::cout << "unknown";
#endif
  std::cout << std::endl;
  std::cout << "  BCL version " << BCL_VERSION_STRING << std::endl;
  std::cout << "  size of data " << Size << std::endl;
  std::cout << "  maximum small size of data " << SmallSize << std::endl;
  std::cout << "  proportion of queues with single or zero elements " <<
    (BoundLessNum ? (BoundLessNum / (double)Size) * 100 : 0) << "%" << std::endl;
  std::cout << "  number of iterations " << MaxIter << std::endl;
  std::map<double, std::string> Time;
  std::cout << std::endl;
  Time.emplace(PushQ.count(), "std::queue construction and push() time (.s) ");
  Time.emplace(PushTQ.count(),
    "bcl::TransparentQueue construction and push() time (.s) ");
  for (auto &T : Time)
    std::cout << T.second << T.first << std::endl;
  std::cout << std::endl;
  Time.clear();
  Time.emplace(PopQ.count(), "std::queue pop() time (.s) ");
  Time.emplace(PopTQ.count(), "bcl::TransparentQueue pop() time (.s) ");
  for (auto &T : Time)
    std::cout << T.second << T.first << std::endl;
  std::cout << std::endl;
  Time.clear();
  Time.emplace(PushSmallQ.count(),
    "small std::queue construction and push() time (.s) ");
  Time.emplace(PushSmallTQ.count(),
    "small bcl::TransparentQueue construction and push() time (.s) ");
  for (auto &T : Time)
    std::cout << T.second << T.first << std::endl;
  std::cout << std::endl;
  Time.clear();
  Time.emplace(PopSmallQ.count(), "small std::queue pop() time (.s) ");
  Time.emplace(PopSmallTQ.count(),
    "small bcl::TransparentQueue pop() time (.s) ");
  for (auto &T : Time)
    std::cout << T.second << T.first << std::endl;
  return 0;
}

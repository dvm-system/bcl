#include "transparent_queue.h"
#include <chrono>
#include <iostream>
#include <queue>
void q_perf () {
  auto S = std::chrono::system_clock::now();
  for (int I = 0; I < 10000; ++I) {
    auto Q = new std::queue<int *>;
    Q->push(&I);
    Q->push(&I);
    Q->push(&I);
    Q->push(&I);
    Q->push(&I);
    Q->push(&I);
    Q->push(&I);
    Q->push(&I);
  }
  auto E = std::chrono::system_clock::now();
  std::chrono::duration<double> Elapsed = E - S;
  std::cout << "std::queue time: " << Elapsed.count() << "s.\n";
}

void tq_perf () {
  auto S = std::chrono::system_clock::now();
  for (int I = 0; I < 10000; ++I) {
   bcl::TransparentQueue<int> TQ;
   TQ.push(&I);
   TQ.push(&I);
   TQ.push(&I);
   TQ.push(&I);
   TQ.push(&I);
   TQ.push(&I);
   TQ.push(&I);
   TQ.push(&I);
  }
  auto E = std::chrono::system_clock::now();
  std::chrono::duration<double> Elapsed = E - S;
  std::cout << "bcl::TransparentQueue time: " << Elapsed.count() << "s.\n";
}

int main() {
  tq_perf();
  q_perf();
  q_perf();
  tq_perf();
  return 0;
}

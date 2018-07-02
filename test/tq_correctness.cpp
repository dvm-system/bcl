#include "transparent_queue.h"
#include <iostream>
#include <string>

template<class Ty> void print(std::string Id, Ty &TQ) {
  auto Tmp(TQ);
  std::cout << "Queue " << Id << ": ";
  while (!Tmp.empty())
    std::cout << *Tmp.pop() << ' ';
  std::cout << std::endl;
}

int main() {
  int I1 = 1, I2 = 2, I3 = 3, I4 = 4;
  bcl::TransparentQueue<int> Q1, Q2;
  std::cout << std::boolalpha <<
    "Empty queue:" <<
    " Q1 == Q2: " << (Q1 == Q2) <<
    " Q1 < Q2: " << (Q1 < Q2) << std::endl;
  print("Q1", Q1);
  print("Q2", Q2);
  Q1.push(&I1);
  Q2.push(&I1);
  std::cout << std::boolalpha <<
    "Identical queue with one element:" <<
    " Q1 == Q2: " << (Q1 == Q2) <<
    " Q1 < Q2: " << (Q1 < Q2) << std::endl;
  print("Q1", Q1);
  print("Q2", Q2);
  auto P2 = new int;
  Q1.push(&I2);
  std::cout << std::boolalpha <<
    "Different queue with two and one elements:" <<
    " Q1 == Q2: " << (Q1 == Q2) <<
    " Q1 < Q2: " << (Q1 < Q2) << std::endl;
  std::cout << std::boolalpha <<
    "Different queue with one and two elements:" <<
    " Q2 == Q1: " << (Q2 == Q1) <<
    " Q2 < Q1: " << (Q2 < Q1) << std::endl;
  print("Q1", Q1);
  print("Q2", Q2);
  Q2.push(&I2);
  std::cout << std::boolalpha <<
    "Identical queue with two elements:" <<
    " Q1 == Q2: " << (Q1 == Q2) <<
    " Q1 < Q2: " << (Q1 < Q2) << std::endl;
  print("Q1", Q1);
  print("Q2", Q2);
  auto P3 = new int;
  auto P4 = new int;
  Q1.push(&I3);
  Q2.push(&I4);
  std::cout << std::boolalpha <<
    "Different queue with three elements:" <<
    " Q1 == Q2: " << (Q1 == Q2) <<
    " Q1 < Q2: " << (Q1 < Q2) << std::endl;
  std::cout << std::boolalpha <<
    "Different queue with three elements:" <<
    " Q2 == Q1: " << (Q2 == Q1) <<
    " Q2 < Q1: " << (Q2 < Q1) << std::endl;
  print("Q1", Q1);
  print("Q2", Q2);
  return 0;
}

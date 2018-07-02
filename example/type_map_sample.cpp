#include <iostream>
#include <typeinfo>
#include <cell.h>

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

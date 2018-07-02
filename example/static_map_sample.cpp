#include <declaration.h>
#include <utility.h>
#include <cell.h>
#include <iostream>

using namespace bcl;

struct Name {
  typedef Base::Text ValueType;
  TO_TEXT( ToText, TEXT( "Name"))
};

struct Salary {
  typedef double ValueType;
  TO_TEXT( ToText, TEXT( "Salary"))
};

typedef StaticMap<Name, Salary> SalaryInfo;

struct PrintStructureFunctor {
  template<class CellTy> void operator()() const {
    typedef typename CellTy::CellKey CellKey;
    std::cout << CellKey::ToText() << " is <value>\n";
  }
};

struct PrintSalaryFunctor {
  template<class CellTy> void operator()(const CellTy *C) const {
    typedef typename CellTy::CellKey CellKey;
    std::cout << CellKey::ToText() << " is ";
    std::cout << C->template value<CellKey>();
    std::cout << std::endl;
  }
};

int main() {
  PrintStructureFunctor PrintStructure;
  std::cout << "Structure of salary information collection:\n"; 
  SalaryInfo::for_each_key(PrintStructure);
  SalaryInfo S;
  S.value<Name>() = TEXT("Smit");
  S.value<Salary>() = 300.50;
  std::cout << "Worker's salary is:\n";
  PrintSalaryFunctor PrintSalary;
  S.for_each(PrintSalary);
  return 1;	
}

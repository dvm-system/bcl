//===--- Equation.h ---------- Affine Equations -----------------*- C++ -*-===//
//
//                       Base Construction Library (BCL)
//
// Copyright 2020 Nikita Kataev
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
// This file implement classes to represent affine equations.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_EQUATION_H
#define BCL_EQUATION_H

#include <assert.h>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>

namespace milp {
/// Monomial of 1 degree `a * x`
template<typename ColumnTy, typename ValueTy>
struct AMonom {
  using ColumnT = ColumnTy;
  using ValueT = ValueTy;

  AMonom() = default;

  AMonom(const ColumnT &C, const ValueT &V) : Column(C), Value(V) {}

  AMonom(const AMonom &) = default;
  AMonom(AMonom &&) = default;

  AMonom &operator=(const AMonom &) = default;
  AMonom &operator=(AMonom &&) = default;

  ColumnT Column;
  ValueT  Value;
};

/// Binomial affine equation of the form `a * x + b * y = c`
template<typename ColumnTy, typename ValueTy>
struct BAEquation {
  using ColumnT = ColumnTy;
  using ValueT = ValueTy;
  using Monom = AMonom<ColumnT, ValueT>;

  BAEquation() = default;

  BAEquation(ColumnT CL, ValueT VL, ColumnT CR, ValueT VR, ValueT C) :
    LHS{ CL, VL }, RHS{ CR, VR }, Constant(C) {}
  BAEquation(Monom L, Monom R, ValueT C) : LHS(L), RHS(R), Constant(C) {}

  BAEquation(const BAEquation &) = default;
  BAEquation(BAEquation &&) = default;
  BAEquation & operator=(const BAEquation &) = default;
  BAEquation & operator=(BAEquation &&) = default;

  Monom LHS;
  Monom RHS;
  ValueT Constant;
};

/// Binomial affine equation with guards and monomials which become known after
/// some computations.
///
/// Equation becomes valid after instantiation which includes evaluation of
/// guards and computation of computable monomials.
///
/// a * x + b * y = c + m1 + ... + mn, g1, ..., gk, ig1, ..., igl
/// - mi is a monomial of the form qi * zi,
/// - gi is a guard which must be evaluated to true,
/// - igi is a guard which must be evaluated to false.
///
/// Any guard may be evaluated to a boolean value and is represented as an
/// object of the ColumnT type. Type of any variable in computable monomial is
/// ColumnT and type of its value is ValueT.
template <typename ColumnT, typename ValueT, std::size_t GuardN,
          std::size_t InverseGuardN, std::size_t ComputedMonomN>
class Row : public BAEquation<ColumnT, ValueT> {
  using GuardList = std::array<ColumnT, GuardN>;
  using InverseGuardList = std::array<ColumnT, InverseGuardN>;
  using ComputedMonomList = std::array<AMonom<ColumnT, ValueT>, ComputedMonomN>;

  template <typename ItrT> class Range {
    ItrT mBegin, mEnd;

  public:
    Range(ItrT B, ItrT E) : mBegin(B), mEnd(E) {}
    ItrT begin() const noexcept { return mBegin; }
    ItrT end() const noexcept { return mEnd; }
  };

public:
  using column_type = ColumnT;
  using value_type = ValueT;

  using guard_iterator = typename GuardList::iterator;
  using guard_range = Range<guard_iterator>;
  using guard_const_iterator = typename GuardList::const_iterator;
  using guard_const_range = Range<guard_const_iterator>;

  Row(ColumnT CL, ValueT VL, ColumnT CR, ValueT VR, ValueT C) :
    BAEquation<ColumnT, ValueT>(CL, VL, CR, VR, C) {}
  Row(AMonom<ColumnT, ValueT> L, AMonom<ColumnT, ValueT> R, ValueT C) :
    BAEquation<ColumnT, ValueT>(L, R, C) {}

  void addGuard(ColumnT Col) noexcept(
      noexcept(std::is_nothrow_assignable<ColumnT, ColumnT>::value)) {
    assert(mGuards.second < GuardN && "Too many guards!");
    mGuards.first[mGuards.second++] = Col;
  }

  guard_iterator guard_begin() noexcept { return mGuards.first.begin(); }
  guard_iterator guard_end() noexcept {
    return mGuards.first.begin() + mGuards.second;
  }
  guard_range guards() noexcept {
    return guard_range{guard_begin(), guard_end()};
  }

  guard_const_iterator guard_begin() const noexcept {
    return mGuards.first.begin();
  }
  guard_const_iterator guard_end() const noexcept {
    return mGuards.first.begin() + mGuards.second;
  }
  guard_const_range guards() const noexcept {
    return guard_const_range{guard_begin(), guard_end()};
  }

  std::size_t guard_size() const noexcept { mGuards.second; }

  using inverse_iterator = typename InverseGuardList::iterator;
  using inverse_range = Range<inverse_iterator>;
  using inverse_const_iterator = typename InverseGuardList::const_iterator;
  using inverse_const_range = Range<inverse_const_iterator>;

  void addInverseGuard(ColumnT Col) noexcept(
      noexcept(std::is_nothrow_assignable<ColumnT, ColumnT>::value)) {
    assert(mInverseGuards.second < InverseGuardN && "Too many inverse guards!");
    mInverseGuards.first[mInverseGuards.second++] = Col;
  }

  inverse_iterator inverse_begin() noexcept {
    return mInverseGuards.first.begin();
  }
  inverse_iterator inverse_end() noexcept {
    return mInverseGuards.first.begin() + mInverseGuards.second;
  }
  inverse_range inverse_guards() noexcept {
    return inverse_range{inverse_begin(), inverse_end()};
  }

  inverse_const_iterator inverse_begin() const noexcept {
    return mInverseGuards.first.begin();
  }
  inverse_const_iterator inverse_end() const noexcept {
    return mInverseGuards.first.begin() + mInverseGuards.second;
  }
  inverse_const_range inverse_guards() const noexcept {
    return inverse_const_range{inverse_begin(), inverse_end()};
  }

  std::size_t inverse_size() const noexcept { mInverseGuards.second; }

  using computed_iterator = typename ComputedMonomList::iterator;
  using computed_range = Range<computed_iterator>;
  using computed_const_iterator = typename ComputedMonomList::const_iterator;
  using computed_const_range = Range<computed_const_iterator>;

  void addComputedMonom(AMonom<ColumnT, ValueT> Monom) noexcept(
      noexcept(std::is_nothrow_assignable<AMonom<ColumnT, ValueT>,
                                          AMonom<ColumnT, ValueT>>::value)) {
    assert(mComputedMonoms.second < ComputedMonomN && "Too many monoms!");
    mComputedMonoms.first[mComputedMonoms.second++] = Monom;
  }

  computed_iterator computed_begin() noexcept {
    return mComputedMonoms.first.begin();
  }
  computed_iterator computed_end() noexcept {
    return mComputedMonoms.first.begin() + mComputedMonoms.second;
  }
  computed_range computed_monoms() noexcept {
    return computed_range{computed_begin(), computed_end()};
  }

  computed_const_iterator computed_begin() const noexcept {
    return mComputedMonoms.first.begin();
  }
  computed_const_iterator computed_end() const noexcept {
    return mComputedMonoms.first.begin() + mComputedMonoms.second;
  }
  computed_const_range computed_monoms() const noexcept {
    return computed_const_range{computed_begin(), computed_end()};
  }

  std::size_t computed_size() const noexcept { mComputedMonoms.second; }

private:
  std::pair<GuardList, std::size_t> mGuards = {{}, 0};
  std::pair<InverseGuardList, std::size_t> mInverseGuards = {{}, 0};
  std::pair<ComputedMonomList, std::size_t> mComputedMonoms = {{}, 0};
};

namespace detail {
template<typename IntT>
std::tuple<IntT, IntT, IntT> euclidGCD(IntT LHS, IntT RHS) {
  if (RHS == 0)
    return std::make_tuple(LHS, 1, 0);
  auto Res = euclidGCD(RHS, LHS % RHS);
  return std::make_tuple(std::get<0>(Res), std::get<2>(Res),
    std::get<1>(Res) - LHS / RHS * std::get<2>(Res));
}
}

/// Compute greatest common divisor for two integer numbers.
///
/// \return GCD and two coefficient A and B such as GCD =  A * LHS + B * RHS
template<typename IntT>
std::tuple<IntT, IntT, IntT> euclidGCD(IntT LHS, IntT RHS) {
  auto Res = detail::euclidGCD(std::abs(LHS), std::abs(RHS));
  if (LHS < 0)
    std::get<1>(Res) = -std::get<1>(Res);
  if (RHS < 0)
    std::get<2>(Res) = -std::get<2>(Res);
  return Res;
}

/// This is a system of binomial affine equations with integer constants.
///
/// Each equation may have guards and computable monomials.
/// To solve the system use
/// - `instantiate()` to instantiate each equation,
/// - `solve()` to solve the system.
///
/// Some methods use objects of ColumnInfoT class which have to provide the
/// following methods:
/// - T get<T>(ColumnT) returns value of a specified variable which is a part of
/// computable monomial (T == ValueT) or it is a guard (T == bool),
/// - ColumnT parameterColumn() returns a new variable which is used to build
/// solution of the system,
/// - ColumnT parameterColumn(ColumnT) returns a new variable which is attached
/// to a specified one, such variables may be useful to match different
/// representations of solutions,
/// bool isParameter(ColumnT) returns true if a specified variable is a
/// parameter that was previously introduces with parameterColumn() methods,
/// - std::string name(ColumnT) returns string representation of a specified
/// variable.
template<typename ColumnT, typename ValueT,
  std::size_t GuardN, std::size_t InverseGuardN, std::size_t ComputedMonomN>
class BinomialSystem {
  using RowT = Row<ColumnT, ValueT, GuardN, InverseGuardN, ComputedMonomN>;
  using EquationT = BAEquation<ColumnT, ValueT>;
  struct UndefT {};

  static constexpr const UndefT Undef{};

  template <typename StreamT>
  void
  log(const char *Message, StreamT &OS,
      typename std::enable_if<!std::is_same<StreamT, const UndefT>::value>::type
          * = nullptr) {
    OS << Message;
  }

  template <typename StreamT>
  void
  log(const char *Message, StreamT &OS,
      typename std::enable_if<std::is_same<StreamT, const UndefT>::value>::type
          * = nullptr) {}

  template <typename ColumnInfoT, typename StreamT>
  void logEquation(
      const EquationT &Row, const ColumnInfoT &Info, StreamT &OS,
      typename std::enable_if<!std::is_same<StreamT, const UndefT>::value>::type
          * = nullptr) {
    printEquation(Row, Info, OS);
    OS << "\n";
  }

  template <typename ColumnInfoT, typename StreamT>
  void logEquation(
      const EquationT &Row, const ColumnInfoT &Info, StreamT &OS,
      typename std::enable_if<std::is_same<StreamT, const UndefT>::value>::type
          * = nullptr) {}

public:
  template <typename ColumnInfoT, typename StreamT>
  static void printEquation(const EquationT &Row, const ColumnInfoT &Info,
                       StreamT &OS) {
    OS << Row.LHS.Value << " " << Info.name(Row.LHS.Column) << " + ";
    OS << Row.RHS.Value << " " << Info.name(Row.RHS.Column) << " = ";
    OS << Row.Constant;
  }

  /// Add new equation to the system.
  void push_back(typename EquationT::Monom LHS, typename EquationT::Monom RHS,
                 ValueT Constant) {
    mRows.emplace_back(LHS, RHS, Constant);
    mIdx.push_back(mRows.size() - 1);
  }

  /// Return the last equation.
  RowT &back() noexcept { return mRows.back(); }

  /// Return number of all equations in the system.
  std::size_t size() const noexcept { return mIdx.size(); }

  /// Return true if the system has been instantiated.
  bool isInstantiated() const noexcept { return mIsInstantiated; }

  /// Return number of equations which were successfully instantiated.
  ///
  /// \pre The system must be instantiated.
  std::size_t instantiated_size() const noexcept {
    assert(isInstantiated() && "System has not been instantiated yet!");
    return mInstantiatedSize;
  }

  using iterator = typename std::vector<RowT>::iterator;
  using const_iterator = typename std::vector<RowT>::const_iterator;

  iterator begin() { return mRows.begin(); }
  iterator end() { return mRows.end(); }

  const_iterator begin() const { return mRows.begin(); }
  const_iterator end() const { return mRows.end(); }

  iterator instantiated_begin() { return mRows.begin(); }
  iterator instantiated_end() { return mRows.begin() + mInstantiatedSize; }

  const_iterator instantiated_begin() const { return mRows.begin(); }
  const_iterator instantiated_end() const {
    return mRows.begin() + mInstantiatedSize;
  }

  /// Solve the instantiated part of the system
  ///
  /// \tparam IsSolvable If it is `true`, assume that the system always has a
  /// solution.
  /// \tparam StreamT Enable logging, if it is specified.
  /// \pre The system was has been instantiated.
  /// \return A number of successfully solved equations.
  template <class ColumnInfoT, bool IsSolvable = true,
            typename StreamT = const UndefT>
  std::size_t solve(ColumnInfoT &Info, StreamT &OS = Undef) {
    for (std::size_t I = 0; I < mInstantiatedSize; ++I) {
      auto &Row = mRows[mIdx[I]];
      // We want to solve binomial equation A * X + B * Y = C
      // 1. Find GCD and two coefficients X' and Y' such as GCD = A * X' + B * Y'
      auto GCD = euclidGCD(Row.LHS.Value, Row.RHS.Value);
      log("> solve:\n", OS);
      logEquation(Row, Info, OS);
      if (!IsSolvable && Row.Constant % std::get<0>(GCD))
        return I;
      assert(Row.Constant % std::get<0>(GCD) == 0 &&
        "Equation must have solution!");
      // 2. It is known that linear equation has solution if GCD of coefficients
      // divides free term. So, we compute Q = C / GCD.
      auto Q = Row.Constant / std::get<0>(GCD);
      // 3.  A * X' + B * Y' = GCD
      //     Q * GCD = C
      //     ----------------------------------------
      // So: A * (X' * Q) + B * (Y' * Q) = GCD *Q = C
      // We find one of possible solutions: (X' * Q, Y' * Q)
      auto AnySolution =
        std::make_pair(Q * std::get<1>(GCD), Q * std::get<2>(GCD));
      auto ParameterCol = Info.parameterColumn();
      // 4. Now, we should solve A * X + B * Y = 0 to find general solution of
      // the original equation. So, we divides this equation by GCD:
      // A' * X + B' * Y = 0
      // One of solutions for this equation is (- B', A')
      // So, the solution of the original equation is:
      // X = (X' * Q) - (B / GCD) * T
      // Y = (Y' * Q) + (A / GCD) * T, where T is any integer value.
      mSolution.emplace_back(Row.LHS.Column, 1,
        ParameterCol, Row.RHS.Value / std::get<0>(GCD), AnySolution.first);
      mSolution.emplace_back(Row.RHS.Column, 1,
        ParameterCol, - Row.LHS.Value / std::get<0>(GCD), AnySolution.second);
      auto &SolutionLHS = mSolution[mSolution.size() - 1];
      auto &SolutionRHS = mSolution[mSolution.size() - 2];
      log("> solution:\n", OS);
      logEquation(SolutionLHS, Info, OS);
      logEquation(SolutionRHS, Info, OS);
      auto updateRow = [](const EquationT &Solution, ValueT &Constant,
          typename RowT::Monom &M) {
        if (M.Column == Solution.LHS.Column) {
          M.Column = Solution.RHS.Column;
          Constant = Constant - Solution.Constant * M.Value;
          M.Value = -(M.Value * Solution.RHS.Value);
        }
      };
      log("> update rows:\n", OS);
      for (std::size_t J = I + 1; J < mInstantiatedSize; ++J) {
        auto &RowToUpdate = mRows[mIdx[J]];
        logEquation(RowToUpdate, Info, OS);
        updateRow(SolutionLHS, RowToUpdate.Constant, RowToUpdate.LHS);
        updateRow(SolutionLHS, RowToUpdate.Constant, RowToUpdate.RHS);
        updateRow(SolutionRHS, RowToUpdate.Constant, RowToUpdate.LHS);
        updateRow(SolutionRHS, RowToUpdate.Constant, RowToUpdate.RHS);
        logEquation(RowToUpdate, Info, OS);
      }
      log("> update solution:\n", OS);
      for (std::size_t J = 0, EJ = mSolution.size() - 2; J < EJ; ++J) {
        auto &SolutionToUpdate = mSolution[J];
        logEquation(SolutionToUpdate, Info, OS);
        updateRow(SolutionLHS, SolutionToUpdate.Constant, SolutionToUpdate.LHS);
        updateRow(SolutionLHS, SolutionToUpdate.Constant, SolutionToUpdate.RHS);
        updateRow(SolutionRHS, SolutionToUpdate.Constant, SolutionToUpdate.LHS);
        updateRow(SolutionRHS, SolutionToUpdate.Constant, SolutionToUpdate.RHS);
        logEquation(SolutionToUpdate, Info, OS);
      }
    }
    // Now, we exclude all equations without original variables.
    std::size_t SignificantSize = mSolution.size();
    for (std::size_t I = 0; I < SignificantSize;) {
      if (Info.isParameter(mSolution[I].LHS.Column) &&
          Info.isParameter(mSolution[I].RHS.Column)) {
        --SignificantSize;
         std::swap(mSolution[I], mSolution[SignificantSize]);
      } else {
        ++I;
      }
    }
    mSolution.resize(SignificantSize);
    return mInstantiatedSize;
  }

  const std::vector<EquationT> &getSolution() const noexcept {
    return mSolution;
  }

  /// Use variables as parameters of solution.
  ///
  /// The method converts
  /// x1 + a1 * t = b1
  /// x2 + a2 * t = b2
  /// to
  /// y1 + c1 * x1 = d1
  /// y2 + c2 * x2 = d2
  template<class ColumnInfoT>
  void reverseSolution(ColumnInfoT &Info) {
    if (mSolution.empty())
      return;
    auto GCD = mSolution[0].RHS.Value;
    for (std::size_t I = 1, EI = mSolution.size(); I < EI; ++I)
      GCD = std::get<0>(euclidGCD(GCD, mSolution[I].RHS.Value));
    std::vector<ValueT> Divisors(mSolution.size());
    for (std::size_t I = 0, EI = mSolution.size(); I < EI; ++I)
      Divisors[I] = mSolution[I].RHS.Value / GCD;
    for (std::size_t I = 0, EI = mSolution.size(); I < EI; ++I) {
      std::swap(Divisors[0], Divisors[I]);
      auto Production = std::accumulate(
        Divisors.begin() + 1, Divisors.end(), 1, std::multiplies<ColumnT>());
      auto &S = mSolution[I];
      Production *= (S.RHS.Value * Production  <  0) ? -1 : 1;
      S.LHS.Value *= Production;
      S.RHS.Value *= Production;
      S.Constant *= Production;
      std::swap(S.LHS, S.RHS);
      S.LHS.Column = Info.parameterColumn(S.LHS.Column);
      S.LHS.Value = 1;
    }
  }

  /// Perform substitutions to ensure that all equations in a previously
  /// computed solution have non-negative free terms.
  template <class ColumnInfoT>
  void solutioWithPositiveConstant(ColumnInfoT &Info) {
    ColumnT Min = 0;
    for (std::size_t I = 0, EI = mSolution.size(); I < EI; ++I) {
      if (mSolution[I].Constant < Min)
        Min = mSolution[I].Constant;
    }
    if (Min == 0)
      return;
    for (std::size_t I = 0, EI = mSolution.size(); I < EI; ++I) {
      auto &S = mSolution[I];
      S.Constant -= Min;
      S.LHS.Column = Info.parameterColumn(S.LHS.Column);
      assert(S.LHS.Value == 1 &&
             "Coefficient for target variable must be one");
    }
  }

  /// Perform instantiation (disable equations with invalid guards and
  /// substitute computable monomials).
  template<class ColumnInfoT>
  void instantiate(const ColumnInfoT &Info) {
    assert(mRows.size() == mIdx.size() && "Storage has been corrupted!");
    assert(!isInstantiated() && "System was already instantiated!");
    mIsInstantiated = true;
    mInstantiatedSize = mIdx.size();
    for (std::size_t I = 0; I < mInstantiatedSize;) {
      auto &Row = mRows[mIdx[I]];
      bool Guard = true;
      for (auto &Col : Row.guards())
        if (!(Guard &= Info.template get<bool>(Col)))
          break;
      if (Guard)
        for (auto &Col : Row.inverse_guards())
          if (!(Guard &= !Info.template get<bool>(Col)))
            break;
      if (Guard) {
        for (auto &Monom : Row.computed_monoms())
          Row.Constant += Monom.Value * Info.template get<ValueT>(Monom.Column);
        ++I;
      } else {
        --mInstantiatedSize;
        std::swap(mIdx[I], mIdx[mInstantiatedSize]);
      }
    }
  }

  template<class ColumnInfoT, class StreamT>
  void printInstantiated(const ColumnInfoT &Info, StreamT &OS) const {
    OS << "--- instantiated ---\n";
    for (std::size_t I = 0; I < mInstantiatedSize; ++I) {
      auto &Row = mRows[mIdx[I]];
      OS << Row.LHS.Value << " " << Info.name(Row.LHS.Column) << " + ";
      OS << Row.RHS.Value << " " << Info.name(Row.RHS.Column) << " = ";
      OS << Row.Constant << " |";
      OS << " guards: ";
      for (auto &Col : Row.guards())
        OS << Info.name(Col) << "=" << Info.template get<bool>(Col) << " ";
      OS << " inverse guards: ";
      for (auto &Col : Row.inverse_guards())
        OS << Info.name(Col) << "=" << Info.template get<bool>(Col) << " ";
      OS << " computed monoms: ";
      for (auto &Monom : Row.computed_monoms())
        OS << Monom.Value << " " << Info.name(Monom.Column) << " = "
         << Monom.Value * Info.template get<ColumnT>(Monom.Column) << " ";
      OS << "\n";
    }
    OS << "--- discarded ---\n";
    for (std::size_t I = mInstantiatedSize, EI = mIdx.size(); I < EI;  ++I) {
      auto &Row = mRows[mIdx[I]];
      OS << Row.LHS.Value << " " << Info.name(Row.LHS.Column) << " + ";
      OS << Row.RHS.Value << " " << Info.name(Row.RHS.Column) << " = ";
      OS << Row.Constant << " |";
      OS << " guards: ";
      for (auto &Col : Row.guards())
        OS << Info.name(Col) << "=" << Info.template get<bool>(Col) << " ";
      OS << " inverse guards: ";
      for (auto &Col : Row.inverse_guards())
        OS << Info.name(Col) << "=" << Info.template get<bool>(Col) << " ";
      OS << " computed monoms: ";
      for (auto &Monom : Row.computed_monoms())
        OS << Monom.Value << " " << Info.name(Monom.Column) << " = "
         << Monom.Value * Info.template get<ColumnT>(Monom.Column) << " ";
      OS << "\n";
    }
  }

  template <class ColumnInfoT, class StreamT>
  void printSolution(const ColumnInfoT &Info, StreamT &OS) {
    OS << "--- solution ---\n";
    for (std::size_t I = 0, EI = mSolution.size(); I < EI; ++I) {
      auto &Solution = mSolution[I];
      OS << Solution.LHS.Value << " " << Info.name(Solution.LHS.Column)
         << " + ";
      OS << Solution.RHS.Value << " " << Info.name(Solution.RHS.Column)
         << " = ";
      OS << Solution.Constant << "\n";
    }
  }

private:
  std::vector<RowT> mRows;
  std::vector<std::size_t> mIdx;
  std::vector<EquationT> mSolution;
  bool mIsInstantiated = false;
  std::size_t mInstantiatedSize = 0;
};

}
#endif//BCL_EQUATION_H

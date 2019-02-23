//===--- convertible_pair.h ----- Convertible Pair --------------*- C++ -*-===//
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
// This file contains a simple wrapper of std::pair which can be implicitly
// converted to a reference to each of the elements.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_CONVERTIBLE_PAIR_H
#define BCL_CONVERTIBLE_PAIR_H

#include <utility>

namespace bcl {
/// This is a simple wrapper of std::pair which can be implicitly converted to
/// a reference to each of the elements.
template<class FirstTy, class SecondTy>
struct convertible_pair : public std::pair<FirstTy, SecondTy> {
  using PairTy = std::pair<FirstTy, SecondTy>;
  template<class... ArgsTy,
    class = typename std::enable_if<
      std::is_constructible<PairTy, ArgsTy&&...>::value>::type>
  constexpr convertible_pair(ArgsTy&&... Args) :
    PairTy(std::forward<ArgsTy>(Args)...) {}
  template<class ArgTy,
    class = typename std::enable_if<
      std::is_constructible<PairTy, ArgTy &&>::value>::type>
  convertible_pair & operator=(ArgTy&& Arg)
  // TODO (kaniandr@gmail.com): add 'noexcept'. Is it possible to use noexcept
  // operator here in a portable way. The following is working on MS VS:
  // 'noexcept(noexcept(PairTy::operator=(std::forward<ArgTy>(Arg))))', however
  // it does not work in GCC. In case of GCC we can use operator= without
  // parameter. However, it seems that it does not work on some OS, for example
  // on FreeBSD.
#ifdef _MSC_VER
  noexcept(noexcept(PairTy::operator=(std::forward<ArgTy>(Arg))))
#endif
  {
    return static_cast<convertible_pair &>(
        PairTy::operator=(std::forward<ArgTy>(Arg)));
  }
  operator FirstTy & () noexcept { return PairTy::first; }
  operator SecondTy & () noexcept { return PairTy::second; }
  operator const FirstTy & () const noexcept { return PairTy::first; }
  operator const SecondTy & () const noexcept { return PairTy::second; }
};
}
#endif//BCL_CONVERTIBLE_PAIR_H

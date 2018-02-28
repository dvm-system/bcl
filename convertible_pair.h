//===--- convertible_pair.h ----- Convertible Pair --------------*- C++ -*-===//
//
//                       Base Construction Library (BCL)
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
  template<class... ArgsTy,
    class = typename std::enable_if<
      std::is_constructible<
        std::pair<FirstTy, SecondTy>, ArgsTy&&...>::value>::type>
  constexpr convertible_pair(ArgsTy&&... Args) :
    std::pair<FirstTy, SecondTy>(std::forward<ArgsTy>(Args)...) {}
  template<class ArgTy,
    class = typename std::enable_if<
      std::is_constructible<
        std::pair<FirstTy, SecondTy>, ArgTy &&>::value>::type>
  convertible_pair & operator=(ArgTy&& Arg) noexcept(noexcept(
    std::pair<FirstTy, SecondTy>::operator=(std::forward<ArgTy>(Arg)))) {
    return static_cast<convertible_pair &>(
        std::pair<FirstTy, SecondTy>::operator=(std::forward<ArgTy>(Arg)));
  }
  operator FirstTy & () noexcept { return first; }
  operator SecondTy & () noexcept { return second; }
  operator const FirstTy & () const noexcept { return first; }
  operator const SecondTy & () const noexcept { return second; }
};
}
#endif//BCL_CONVERTIBLE_PAIR_H

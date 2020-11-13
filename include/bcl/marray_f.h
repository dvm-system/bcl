//===--- marray_f.h --------- Multidimensional Array ------------*- C++ -*-===//
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
// This file implements multidimensional array. Each element if this array can
// be accessed using operator().
// The example below creates an array A with 3 dimensions (10x10x10) and
// assign the 5 constant to an element with indexes (1, 2, 3).
//   bcl::marray<int, 3> A{{10, 10, 10}};
//   A(1, 2, 3) = 5;
//
//===----------------------------------------------------------------------===//

#ifndef BCL_MARRAY_F_H
#define BCL_MARRAY_F_H

#include <bcl/utility.h>
#include <array>

namespace bcl {
/// Represent a multidimensional array.
///
/// \tparam Ty Type of each element.
/// \tparam Size Number of dimensions.
template<class Ty, size_t Size>
class marray_f {
  /// Allocate memory according to sizes stored in mDims.
  void allocate() {
    auto FullSize = mDims[Size - 1];
    mOffset[Size - 2] = mDims[Size - 1];
    for (int I = Size - 2; I > 0; --I) {
      FullSize *= mDims[I];
      mOffset[I-1] = mOffset[I] * mDims[I];
    }
    FullSize *= mDims[0];
    mData = new Ty [FullSize];
  }

public:
  /// Create an array with specified sizes of dimensions.
  explicit inline marray_f(const std::array<size_t, Size> &Dims)
      BCL_ALWAYS_INLINE : mDims(Dims) {
    allocate();
  }

  /// Create an array with specified sizes of dimensions.
  explicit inline marray_f(std::array<size_t, Size> &&Dims)
      BCL_ALWAYS_INLINE : mDims(std::move(Dims))   {
   allocate();
  }

  marray_f(const marray_f &) = delete;
  marray_f & operator=(const marray_f &) = delete;

  inline marray_f(marray_f &&From) BCL_ALWAYS_INLINE :
      mDims(std::move(From.mDims)),
      mOffset(std::move(From.mOffset)),
      mData(From.mData) {
    From.mData = nullptr;
  }

  inline marray_f & operator=(marray_f &&From) BCL_ALWAYS_INLINE {
    mDims = std::move(From.mDims);
    mOffset = std::move(From.mOffset);
    mData = From.mData;
    From.mData = nullptr;
  }

  inline ~marray_f() BCL_ALWAYS_INLINE {
    if (mData)
      delete [] mData;
  }

  /// Return a reference to a specified element.
  template<class... Args> Ty & operator()(Args ...A) {
    return *offset(mData, 0, A...);
  }

  /// Return a reference to a specified element.
  template<class... Args> const Ty & operator()(Args ...A) const {
    return *offset(mData, 0, A...);
  }

private:
/// Recursively calculates offset to access an element, end of recursion.
  template<class Last>
  Ty * offset(Ty *Offset, size_t, Last A) const noexcept {
    return Offset + A;
  }

  /// Recursively calculates offset to access an element.
  template<class First, class... Args>
  Ty * offset(Ty *Offset, size_t Dim, First F, Args... A ) const noexcept {
    return offset(Offset + mOffset[Dim] * F, Dim + 1, A...);
  }

private:
  Ty *mData;
  std::array<size_t, Size> mDims;
  std::array<size_t, Size - 1> mOffset;
};
}
#endif//BCL_MARRAY_F_H

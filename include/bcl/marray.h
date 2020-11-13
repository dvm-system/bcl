//===--- marray.h ----------- Multidimensional Array ------------*- C++ -*-===//
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
// be accessed using operator[].
// The example below creates an array A with 3 dimensions (10x10x10) and
// assign the 5 constant to an element with indexes (1, 2, 3).
//   bcl::marray<int, 3> A{{10, 10, 10}};
//   A[1][2][3] = 5;
//
//===----------------------------------------------------------------------===//

#ifndef BCL_MARRAY_H
#define BCL_MARRAY_H

#include <bcl/utility.h>
#include <array>

namespace bcl {
template<typename Ty, std::size_t Size, typename marray>
class msubarray {
public:
  inline msubarray(Ty *Data, marray *Base) noexcept BCL_ALWAYS_INLINE :
    mData(Data), mBase(Base) {}

  msubarray<Ty, Size - 1, marray> operator[](std::size_t I) {
    return msubarray<Ty, Size - 1, marray>(
      mData + mBase->mOffset[mBase->mOffset.size() - Size + 1] * I, mBase);
  }
private:
  Ty *mData;
  marray *mBase;
};

template<typename Ty, typename marray>
class msubarray<Ty, 1, marray> {
public:
  inline msubarray(Ty *Data, marray *) noexcept BCL_ALWAYS_INLINE :
    mData(Data){}

  const Ty & operator[](std::size_t I) const { return *(mData + I); }

  Ty & operator[](std::size_t I) { return *(mData + I); }
private:
  Ty *mData;
};

/// Represent a multidimensional array.
///
/// \tparam Ty Type of each element.
/// \tparam Size Number of dimensions.
template<typename Ty, std::size_t Size>
class marray {
  template<typename T, std::size_t S, typename MA>
  friend class msubarray;
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
  explicit inline marray(const std::array<std::size_t, Size> &Dims)
      BCL_ALWAYS_INLINE : mDims(Dims) {
    allocate();
  }

  /// Create an array with specified sizes of dimensions.
  explicit inline marray(std::array<std::size_t, Size> &&Dims)
      BCL_ALWAYS_INLINE : mDims(std::move(Dims)) {
   allocate();
  }

  marray(const marray &) = delete;
  marray & operator=(const marray &) = delete;

  inline marray(marray &&From) BCL_ALWAYS_INLINE :
      mDims(std::move(From.mDims)),
      mOffset(std::move(From.mOffset)),
      mData(From.mData) {
    From.mData = nullptr;
  }

  inline marray & operator=(marray &&From) BCL_ALWAYS_INLINE {
    mDims = std::move(From.mDims);
    mOffset = std::move(From.mOffset);
    mData = From.mData;
    From.mData = nullptr;
  }

  inline ~marray() BCL_ALWAYS_INLINE {
    delete [] mData;
  }

  /// Return a subarray with Size - 1 number of dimensions.
  msubarray <Ty, Size - 1, marray> operator[](std::size_t I) {
    return msubarray<Ty, Size - 1, marray>(mData + mOffset[0] * I, this);
  }

private:
  Ty *mData;
  std::array<std::size_t, Size> mDims;
  std::array<std::size_t, Size - 1> mOffset;
};
}
#endif//BCL_MARRAY_H

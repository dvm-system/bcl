//===- IteratorDataAdaptor.h --  --------------*- C++ -*-===//
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
// This file contains a simple wrapper of a pair of iterator and some value.
// It behaves as a wrapped iterator but has attached data.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_ITERATOR_DATA_ADAPTOR_H
#define BCL_ITERATOR_DATA_ADAPTOR_H

#include <iterator>
#include <type_traits>

namespace bcl {
///\brief A simple wrapper of a pair of iterator and some value.
///
/// This behaves as a wrapped iterator but has attached data.
/// \tparam ItrTy Type of a wrapped iterator.
/// \tparam DataTy Type of attached data.
/// \tparam Ty Type of a value wrapper points to. It should be constructible
/// from *ItrTy, DataTy pair.
/// \tparam PtrTy Type of a pointer to a type of a value wrapper points to.
/// Note, that there is no `operator->` method and `operator*` return a value
/// instead of a reference.
template<class ItrTy, class DataTy, class Ty, class PtrTy = Ty *>
struct IteratorDataAdaptor : public std::iterator<
      typename std::iterator_traits<ItrTy>::iterator_category,
      typename std::iterator_traits<ItrTy>::difference_type, Ty, PtrTy, Ty> {
  static_assert(std::is_constructible<Ty,
    typename std::iterator_traits<ItrTy>::reference, DataTy>::value,
    "Wrapper must point to a value which is constructible from (*ItrTy, DataTy)!");

  IteratorDataAdaptor(const ItrTy &I, const DataTy &D) : mItr(I), mData(D) {}

  bool operator==(const IteratorDataAdaptor &RHS) const {
    return mItr == RHS.mItr && mData == RHS.mData;
  }
  bool operator!=(const IteratorDataAdaptor &RHS) const {
    return !operator==(RHS);
  }

  IteratorDataAdaptor & operator++() { ++mItr; return *this; }
  IteratorDataAdaptor operator++(int) {
    auto Tmp = *this; ++*this; return Tmp;
  }

  Ty operator*() const { return Ty(*mItr, mData); }

  ItrTy & getIterator() noexcept { return mItr; }
  const ItrTy & getIterator() const noexcept { return mItr; }

  DataTy & getData() noexcept { return mData; }
  const DataTy & getData() const noexcept { return mData; }

private:
  ItrTy mItr;
  DataTy mData;
};
}
#endif//BCL_ITERATOR_DATA_ADAPTOR_H

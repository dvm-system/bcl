//===----- ValuePtrWrapper.h --- Value Wrappers -----------------*- C++ -*-===//
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
// This file defines value wrappers.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_VALUE_PTR_WRAPPER_H
#define BCL_VALUE_PTR_WRAPPER_H

namespace bcl {
/// This is a wrapper for a value of a specified type `T` which knows a pointer
/// referenced this value.
///
/// This wrapper updates a known pointer if location of a value is changed.
/// Note, that this wrapper is uncopyable, so move operations could change
/// value location only.
///
/// \code
/// std::vector<ValuePtrWrapper<T>> Pool;
/// T *Ptr;
/// Pool.emplace_back(Ptr, Arg1, ..., ArgN);
/// \endcode
/// So, a new object T(Arg1, ..., ArgN) is stored in a Pool, and Ptr always
/// points to this object.
template<class T> struct ValuePtrWrapper {
  template<class... ArgT>
  explicit ValuePtrWrapper(T *&Ptr, ArgT... Args) :
    mValue(std::forward<ArgT>(Args)...), mPtr(&Ptr) {
    *mPtr = &mValue;
  }
  ValuePtrWrapper(ValuePtrWrapper &) = delete;
  ValuePtrWrapper & operator=(ValuePtrWrapper &) = delete;
  ValuePtrWrapper(ValuePtrWrapper &&From) :
    mValue(std::move(From.mValue)), mPtr(From.mPtr) {
    *mPtr = &mValue;
  }
  ValuePtrWrapper & operator=(ValuePtrWrapper &&From) {
    mValue = std::move(From.mValue);
    mPtr = From.mPtr;
    *mPtr = &mValue;
    return *this;
  }

  operator T &() noexcept { return mValue; }
  operator const T &() const noexcept { return mValue; }
  T * operator &() { return &mValue; }
  T & operator *() { return mValue; }
  T * operator-> () { return &mValue; }

private:
  T mValue;
  T **mPtr;
};
}



#endif//BCL_VALUE_PTR_WRAPPER_H

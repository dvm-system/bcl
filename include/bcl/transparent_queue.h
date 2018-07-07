//===-- tsar_transparent_queue.h - Transparent Queue ------------*- C++ -*-===//
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
// This file implements a special queue of pointers. In case of single element
// in a queue no memory allocation is required.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_TRANSPARENT_QUEUE_H
#define BCL_TRANSPARENT_QUEUE_H

#include <assert.h>
#include <cstring>
#include <queue>

namespace bcl {
/// \brief Simple queue that store pointers.
///
/// This can be in two states. At first this contains only one element which is
/// stored as a pointer. But if the second element will be inserted the internal
/// data storage will be transparently converted to a queue of elements.
///
/// This class is convenient to create some queue which usually contains only
/// one element.
template<class Ty>
class TransparentQueue {
  typedef std::queue<Ty *> Container;
public:
  typedef typename Container::value_type value_type;
  typedef typename Container::size_type size_type;
  typedef typename Container::reference reference;
  typedef typename Container::const_reference const_reference;

  /// Creates an empty queue.
  TransparentQueue() noexcept :
    mIsSingle(true), mIsEmpty(true), mValue(nullptr) {}

  /// Creates a queue that contains one pointer.
  explicit TransparentQueue(value_type V) noexcept :
    mIsSingle(true), mIsEmpty(false), mValue(V) {}

  /// Copy constructor.
  TransparentQueue(const TransparentQueue &TQ) :
      mIsSingle(TQ.mIsSingle), mIsEmpty(TQ.mIsEmpty) {
    if (mIsSingle)
      mValue = TQ.mValue;
    else
      mQueue = new Container(*TQ.mQueue);
  };

  /// Copy assignment.
  TransparentQueue & operator=(const TransparentQueue &TQ) {
    mIsSingle = TQ.mIsSingle;
    mIsEmpty = TQ.mIsEmpty;
    if (mIsSingle)
      mValue = TQ.mValue;
    else
      mQueue = new Container(*TQ.mQueue);
  }

  /// Move constructor.
  TransparentQueue(TransparentQueue &&TQ) :
      mIsSingle(TQ.mIsSingle), mIsEmpty(TQ.mIsEmpty) {
    std::memmove(&mQueue, &TQ.mQueue, sizeof(TQ.mQueue));
    TQ.mIsSingle = true;
    TQ.mIsEmpty = true;
  }

  /// Move assignment.
  TransparentQueue & operator=(TransparentQueue &&TQ) {
    mIsSingle = TQ.mIsSingle;
    mIsEmpty = TQ.mIsEmpty;
    std::memmove(&mQueue, &TQ.mQueue, sizeof(TQ.mQueue));
    TQ.mIsSingle = true;
    TQ.mIsEmpty = true;
  }

  /// Removes allocated memory if it is necessary.
  ~TransparentQueue() {
    if (!mIsSingle) {
      assert(mQueue && "An internal storage must not be null!");
      delete mQueue;
    }
  }

  /// Inserts an element V at the end of this queue.
  void push(value_type V) {
    if (mIsSingle) {
      if (mIsEmpty) {
        mIsEmpty = false;
        mValue = V;
        return;
      }
      mIsSingle = false;
      auto Current = mValue;
      mQueue = new Container();
      mQueue->push(Current);
    }
    mQueue->push(V);
  }

  /// Removes element at the beginning of this queue.
  value_type pop() {
    if (mIsSingle) {
      auto V = mValue;
      mValue = nullptr;
      mIsEmpty = true;
      return V;
    }
    if (mQueue->empty())
      return nullptr;
    auto V = mQueue->front();
    mQueue->pop();
    return V;
  }

  /// Returns number of elements in this queue.
  size_type size() const {
    return mIsSingle ? mIsEmpty ? 0 : 1 : mQueue->size();
  }

  /// Returns true if the queue is empty.
  bool empty() const {
    return mIsSingle ? mIsEmpty : mQueue->empty();
  }

  /// Returns reference to the first element in the queue.
  reference front() { return mIsSingle ? mValue: mQueue->front(); }

  /// Returns reference to the first element in the queue.
  const_reference front() const { return mIsSingle ? mValue : mQueue->front(); }

  /// Returns reference to the last element in the queue.
  reference back() { return mIsSingle ? mValue : mQueue->back(); }

  /// Returns reference to the last element in the queue.
  const_reference back() const { return mIsSingle ? mValue : mQueue->back(); }

  /// Pushes new element to the end of the queue.
  template<class... ArgTy> void emplace(ArgTy&&... Args) {
    if (mIsSingle) {
      mIsSingle = false;
      Ty *Current = mValue;
      mQueue = new Container();
      mQueue->push(Current);
    }
    mQueue->emplace(Args...);
  }

  /// \brief Exchanges the contents of the queue with those of other.
  ///
  /// TODO (kaniandr@gmail.com) : implement noexcept specification, note that
  /// the following one does not work:
  /// \code
  /// noexcept(
  ///   noexcept(std::swap(mValue, TQ.mValue)) &&
  ///   noexcept(mQueue->swap(TQ.mQueue))) {
  /// \endcode
  void swap(TransparentQueue &TQ) {
    if (mIsSingle) {
      if (TQ.mIsSingle) {
        std::swap(mValue, TQ.mValue);
        std::swap(mIsEmpty, TQ.mIsEmpty);
      } else {
        Ty *Tmp = mValue;
        mIsSingle = false;
        mQueue = TQ.mQueue;
        TQ.mIsSingle = true;
        TQ.mIsEmpty = mIsEmpty;
        TQ.mValue = Tmp;
      }
    } else {
      if (TQ.mIsSingle) {
        Ty *Tmp = TQ.Value;
        TQ.mIsSingle = false;
        TQ.mQueue = mQueue;
        mIsSingle = true;
        mIsEmpty = TQ.mIsEmpty;
        mValue = Tmp;
      } else {
        mQueue->swap(TQ.mQueue);
      }
    }
  }

  /// Compares the contents of two containers.
  bool operator==(const TransparentQueue &TQ) const {
    if (size() != TQ.size())
      return false;
    if (mIsSingle) {
      if (TQ.mIsSingle)
        return mIsEmpty == TQ.mIsEmpty ?
          mIsEmpty ? true : mValue == TQ.mValue : false;
      else
        return mIsEmpty == TQ.mQueue->empty() ?
          mIsEmpty ? true : mValue == TQ.mQueue->front() : false;
    } else {
      if (TQ.mIsSingle)
        return TQ.mIsEmpty == mQueue->empty() ?
          TQ.mIsEmpty ? true : TQ.mValue == mQueue->front() : false;
      else
        return *mQueue == *TQ.mQueue;
    }
  }

  /// Lexicographically compares the values in the queue.
  bool operator<(const TransparentQueue &TQ) const {
    if (mIsSingle == TQ.mIsSingle) {
      return mIsSingle ? mIsEmpty == TQ.mIsEmpty ?
        (mIsEmpty ? false : mValue < TQ.mValue) :
        (mIsEmpty ? true : false) :
        *mQueue < *TQ.mQueue;
    } else {
      if (mIsSingle) {
        Container Tmp;
        if (!mIsEmpty)
          Tmp.push(mValue);
        return Tmp < *TQ.mQueue;
      }
      else {
        Container Tmp;
        if (!TQ.mIsEmpty)
          Tmp.push(TQ.mValue);
        return *mQueue < Tmp;
      }
    }
  }

private:
  struct {
    bool mIsSingle : 1;
    bool mIsEmpty : 1;
  };

  union {
    value_type mValue;
    Container *mQueue;
  };
};

/// Exchanges the contents of the queue with those of other.
template<class Ty> inline
void swap(TransparentQueue<Ty> &Left, TransparentQueue<Ty> &Right)
    noexcept(noexcept(Left.swap(Right))) {
  Left.swap(Right);
}

/// Compares the contents of two queues.
template<class Ty> inline bool operator!=(
    const TransparentQueue<Ty> &Left, const TransparentQueue<Ty> &Right) {
  return !(Left == Right);
}

/// Lexicographically compares the values in the queue.
template<class Ty> inline bool operator>(
  const TransparentQueue<Ty> &Left, const TransparentQueue<Ty> &Right) {
  return Right < Left;
}

/// Lexicographically compares the values in the queue.
template<class Ty> inline bool operator<=(
  const TransparentQueue<Ty> &Left, const TransparentQueue<Ty> &Right) {
  return !(Right < Left);
}

/// Lexicographically compares the values in the queue.
template<class Ty> inline bool operator>=(
  const TransparentQueue<Ty> &Left, const TransparentQueue<Ty> &Right) {
  return !(Left < Right);
}
}

#endif//BCL_TRANSPARENT_QUEUE_H

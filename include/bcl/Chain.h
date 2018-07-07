//===--- Chain.h ---------- Simple Intrusive Chain -------------*- C++ -*-===//
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
// This file implements a simple intrusive chain of nodes. A chain is intrusive
// because each node should contain links to next and previous nodes. To do this
/// each node in a chain should inherit Chain class. A special Tag field can be
// used if node wants to be included in multiple chains. To traverse and update
// a chain ChainTraits and ChainIterator[C] classes can be used.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_CHAIN_H
#define BCL_CHAIN_H

namespace bcl {
/// \brief Simple bidirectional intrusive chain of nodes.
///
/// It is necessary to inherit this class to implement a chain of nodes.
/// To traverse nodes in a chain and to update this chain bcl::ChainTraits and
/// bcl::ChainIterator[C] classes can be used.
/// \tparam Ty Type of each node, it must inherit this class.
/// \tparam Tag Identifier of a chain, a single node can inherit multiple Chain
/// classes with different identifiers. In this case this node will be treated
/// as a node in different chains.
template<class Ty, class Tag = void> class Chain {
public:
  /// Implicit cast to a data type Ty.
  operator Ty & () noexcept {
    return static_cast<Ty &>(*this);
  }

  /// Implicit cast to a data type Ty.
  operator const Ty & () const noexcept {
    return static_cast<const Ty &>(*this);
  }

private:
  /// \brief Sets a specified node as a next node for this one.
  ///
  /// The node N will be extracted from its owner and insert into this chain.
  void spliceNext(Chain *N) noexcept {
    assert(N != this && "A node must not follow itself!");
    if (N) {
      if (N->mNext)
        N->mNext->mPrev = nullptr;
      if (N->mPrev)
        N->mPrev->mNext = nullptr;
      N->mNext = mNext;
      N->mPrev = this;
    }
    if (mNext)
      mNext->mPrev = N;
    mNext = N;
  }

  /// \brief Sets a specified node as a next node for this one.
  ///
  /// The node N will be treated as a start node of chain which should be
  /// merged to this node.
  void mergeNext(Chain *N) noexcept {
    assert(N != this && "A node must not follow itself!");
    if (N) {
      if (N->mPrev)
        N->mPrev->mNext = nullptr;
      N->mPrev = this;
    }
    if (mNext)
      mNext->mPrev = nullptr;
    mNext = N;
  }

  /// \brief Sets a specified node as a previous node for this one.
  ///
  /// The node N will be extracted from its owner and insert into this chain.
  void splicePrev(Chain *N) noexcept {
    assert(N != this && "A node must not precede itself!");
    if (N) {
      if (N->mNext)
        N->mNext->mPrev = nullptr;
      if (N->mPrev)
        N->mPrev->mNext = nullptr;
      N->mPrev = mPrev;
      N->mNext = this;
    }
    if (mPrev)
      mPrev->mNext = N;
    mPrev = N;
  }

  /// \brief Sets a specified node as a previous node for this one.
  ///
  /// The node N will be treated as a start node of chain which should be
  /// merged to this node.
  void mergePrev(Chain *N) noexcept {
    assert(N != this && "A node must not precede itself!");
    if (N) {
      if (N->mNext)
        N->mNext->mPrev = nullptr;
      N->mNext = this;
    }
    if (mPrev)
      mPrev->mNext = nullptr;
    mPrev = N;
  }

  /// Returns a next node.
  Ty * getNext() noexcept { return static_cast<Ty *>(mNext); }

  /// Returns a next node.
  const Ty * getNext() const noexcept { return static_cast<const Ty *>(mNext); }

  /// Returns a previous node.
  Ty * getPrev() noexcept { return static_cast<Ty *>(mPrev); }

  /// Returns a previous node.
  const Ty * getPrev() const noexcept { return static_cast<const Ty *>(mPrev); }

private:
  template<class, class> friend struct ChainTraits;
  template<class, class> friend struct ChainIterator;

  Chain *mPrev = nullptr;
  Chain *mNext = nullptr;
};

/// Proposes methods to traverse and to update chains.
template<class Ty, class Tag = void> struct ChainTraits {
  static_assert(std::is_base_of<Chain<Ty, Tag>, Ty>::value,
    "Each node in a chain must inherit chain implementation!");

  /// \brief Sets a specified node as a next node for N.
  ///
  /// The node Next will be extracted from its owner and insert into this chain.
  static void spliceNext(Chain<Ty, Tag> *Next, Chain<Ty, Tag> *N)
    noexcept(noexcept(N->spliceNext(Next))) {
    assert(N && "Chain must not be null!");
    N->spliceNext(Next);
  }

  /// \brief Sets a specified node as a next node for N.
  ///
  /// The node Next will be treated as a start node of chain which should be
  /// merged to this node.
  static void mergeNext(Chain<Ty, Tag> *Next, Chain<Ty, Tag> *N)
    noexcept(noexcept(N->mergeNext(Next))) {
    assert(N && "Chain must not be null!");
    N->mergeNext(Next);
  }

  /// \brief Sets a specified node as a previous node for N.
  ///
  /// The node Prev will be extracted from its owner and insert into this chain.
  static void splicePrev(Ty *Prev, Chain<Ty, Tag> *N)
      noexcept(noexcept(N->splicePrev(Prev))) {
    assert(N && "Chain must not be null!");
    N->splicePrev(Prev);
  }

  /// \brief Sets a specified node as a previous node for N.
  ///
  /// The node Prev will be treated as a start node of chain which should be
  /// merged to this node.
  static void mergePrev(Ty *Prev, Chain<Ty, Tag> *N)
      noexcept(noexcept(N->mergePrev(Prev))) {
    assert(N && "Chain must not be null!");
    N->mergePrev(Prev);
  }

  /// Returns a next node.
  static Ty * getNext(Chain<Ty, Tag> *N)
      noexcept(noexcept(N->getNext())) {
    assert(N && "Node must not be null!");
    return N->getNext();
  }

  /// Returns a next node.
  static const Ty * getNext(const Chain<Ty, Tag> *N)
      noexcept(noexcept(N->getNext())) {
    assert(N && "Node must not be null!");
    return N->getNext();
  }

  /// Returns a previous node.
  static Ty * getPrev(Chain<Ty, Tag> *N)
      noexcept(noexcept(N->getPrev())) {
    assert(N && "Node must not be null!");
    return N->getPrev();
  }

  /// Returns a previous node.
  static const Ty * getPrev(const Chain<Ty, Tag> *N)
      noexcept(noexcept(N->getPrev())) {
    assert(N && "Node must not be null!");
    return N->getPrev();
  }
};

/// This type is used to iterate over all nodes in a chain,
/// suffix 'C' specifies that this is a constant iterator over a constant chain
/// with constant nodes.
template<class Ty, class Tag = void> class ChainIteratorC :
  public std::iterator<std::bidirectional_iterator_tag, const Ty> {
  static_assert(std::is_base_of<Chain<Ty, Tag>, Ty>::value,
    "Each node in a chain must inherit chain implementation!");
public:
  using value_type = typename std::iterator<std::bidirectional_iterator_tag, const Ty>::value_type;

  /// Implicitly create an iterator, which starts at a specified node.
  ChainIteratorC(const Chain<Ty, Tag> *C) noexcept : mCurrent(C) {}

  /// Update an iterator, so it will start at a specified node.
  ChainIteratorC & operator=(const Chain<Ty, Tag> *C) noexcept {
    mCurrent = C;
    return *this;
  }

  /// Checks whether this iterator points to something.
  operator bool() const noexcept { return mCurrent != nullptr; }

  /// Compares two iterators.
  bool operator==(const ChainIteratorC &RHS) const noexcept {
    return mCurrent == RHS.mCurrent;
  }

  /// Compares two iterators.
  bool operator!=(const ChainIteratorC& RHS) const noexcept {
    return !operator==(RHS);
  }

  /// Compares the iterator with a node.
  bool operator==(const Ty *RHS) const noexcept {
    mCurrent == RHS;
  }

  /// Compares the iterator with a node.
  bool operator!=(const Ty *RHS) const noexcept {
    return !operator==(RHS);
  }

  /// Dereferences this iterator.
  value_type & operator*() const noexcept {
    assert(mCurrent && "Dereferencing null node!");
    return *mCurrent;
  }

  /// Dereferences this iterator.
  value_type * operator->() const noexcept { return &operator*(); }

  /// Preincrement, this node must not be null.
  ChainIteratorC & operator++()
      noexcept(noexcept(ChainTraits<Ty, Tag>::getNext())) {
    assert(mCurrent && "Advancing null node!");
    mCurrent = ChainTraits<Ty, Tag>::getNext(mCurrent);
    return *this;
  }

  /// Postincrement, this node must not be null.
  ChainIteratorC operator++(int)
      noexcept(noexcept(ChainIteratorC::operator++())) {
    ChainIteratorC Tmp = *this; ++*this; return Tmp;
  }

  /// Predecrement, this node must not be null.
  ChainIteratorC & operator--()
      noexcept(noexcept(ChainTraits<Ty, Tag>::getPrev())) {
    assert(mCurrent && "Decreasing null node!!");
    mCurrent = ChainTraits<Ty, Tag>::getPrev(mCurrent);
    return *this;
  }

  /// Postdecrement, this node must not be null.
  ChainIteratorC operator--(int)
      noexcept(noexcept(ChainIteratorC::operator--())) {
    ChainIteratorC Tmp = *this; ++*this; return Tmp;
  }

  /// Returns a current node.
  const Ty * get() const noexcept { return &operator*(); }

  /// Returns a next node.
  const Ty * getNext() const
      noexcept(noexcept(ChainTraits<Ty, Tag>::getNext())) {
    return ChainTraits<Ty, Tag>::getNext(mCurrent);
  }

  /// Returns a previous node.
  const Ty * getPrev() const
      noexcept(noexcept(ChainTraits<Ty, Tag>::getPrev())) {
    return ChainTraits<Ty, Tag>::getPrev(mCurrent);
  }

private:
  const Chain<Ty, Tag> *mCurrent;
};

/// This type is used to iterate over all nodes in a chain.
template<class Ty, class Tag = void> class ChainIterator :
  public ChainIteratorC<Ty, Tag> {
  typedef ChainIteratorC<Ty, Tag> Base;
public:
  typedef Ty value_type;
  typedef Ty & reference;
  typedef Ty * pointer;

  /// Implicitly create an iterator, which starts at a specified node.
  ChainIterator(Chain<Ty, Tag> *C) noexcept : Base(C) {}

  /// Update an iterator, so it will start at a specified node.
  ChainIterator & operator=(Chain<Ty, Tag> *C) noexcept {
    Base::operator=(C);
    return *this;
  }

  /// Dereferences this iterator.
  value_type & operator*() const noexcept {
    return const_cast<value_type &>(Base::operator*());
  }

  /// Dereferences this iterator.
  value_type * operator->() const noexcept {
    return const_cast<value_type *>(Base::operator->());
  }

  /// Preincrement, this node must not be null.
  ChainIterator & operator++()
      noexcept(noexcept(Base::operator++())) {
    Base::operator++();
    return *this;
  }

  /// Postincrement, this node must not be null.
  ChainIterator operator++(int)
      noexcept(noexcept(ChainIterator::operator++())) {
    ChainIterator Tmp = *this; ++*this; return Tmp;
  }

  /// Predecrement, this node must not be null.
  ChainIterator & operator--()
      noexcept(noexcept(Base::operator--())) {
    Base::operator--();
    return *this;
  }

  /// Postdecrement, this node must not be null.
  ChainIterator operator--(int)
      noexcept(noexcept(ChainIterator::operator--())) {
    ChainIterator Tmp = *this; ++*this; return Tmp;
  }

  /// Returns a current node.
  Ty * get() const noexcept { return const_cast<Ty *>(Base::get()); }

  /// Returns a next node.
  Ty * getNext() const
      noexcept(noexcept(Base::getNext())) {
    return const_cast<Ty *>(Base::getNext());
  }

  /// Returns a previous node.
  Ty * getPrev() const
      noexcept(noexcept(Base::getPrev())) {
    return const_cast<Ty *>(Base::getPrev());
  }

  /// \brief Sets a specified node as a next node for this one.
  ///
  /// The node N will be extracted from its owner and insert into this chain.
  void spliceNext(Ty *N) const
      noexcept(noexcept(ChainTraits<Ty, Tag>::spliceNext(N, get()))) {
    ChainTraits<Ty, Tag>::spliceNext(N, get());
  }

  /// \brief Sets a specified node as a next node for this one.
  ///
  /// The node N will be treated as a start node of chain which should be
  /// merged to this node.
  void mergeNext(Ty *N) const
      noexcept(noexcept(ChainTraits<Ty, Tag>::mergeNext(N, get()))) {
    ChainTraits<Ty, Tag>::mergeNext(N, get());
  }

  /// \brief Sets a specified node as a previous node for this one.
  ///
  /// The node N will be extracted from its owner and insert into this chain.
  void splicePrev(Ty *N) const
      noexcept(noexcept(ChainTraits<Ty, Tag>::splicePrev(N, get()))) {
    ChainTraits<Ty, Tag>::splicePrev(N, get());
  }

  /// \brief Sets a specified node as a previous node for this one.
  ///
  /// The node N will be treated as a start node of chain which should be
  /// merged to this node.
  void mergePrev(Ty *N) const
      noexcept(noexcept(ChainTraits<Ty, Tag>::mergePrev(N, get()))) {
    ChainTraits<Ty, Tag>::mergePrev(N, get());
  }
};

/// Compares a node with an iterator.
template<class Ty, class Tag>
bool operator==(Ty *LHS, const ChainIteratorC<Ty, Tag> &RHS) noexcept {
  return RHS == LHS;
}

/// Compares a node with an iterator.
template<class Ty, class Tag>
bool operator!=(Ty *LHS, const ChainIteratorC<Ty, Tag> &RHS) noexcept {
  return RHS != LHS;
}
}
#endif//BCL_CHAIN_H

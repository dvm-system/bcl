//===--- Diagnostic.h --- Simple Diagnostics Container ----------*- C++ -*-===//
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
// This file implements a simple container for diagnostics (warnings,
// errors, etc.).
// All diagnostics are represented as a string: <Kind> C<Code>(<Pos>): <Fmt>,
// for example: error C101(100): unexpected character 'c'.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_DIAGNOSTIC_H
#define BCL_DIAGNOSTIC_H

#include <cstring>
#include <memory>
#include <vector>

namespace bcl {
/// \brief This is a simple container for diagnostics (warnings, errors, etc.).
///
/// All diagnostics are represented as a character string.
class Diagnostic {
  typedef std::vector<std::unique_ptr<const char>> Collection;

public:
  typedef const char * value_type;
  typedef value_type reference;
  typedef const value_type const_reference;
  typedef value_type * pointer;
  typedef const value_type * const_pointer;

private:
  /// Bidirectional iterator which is a wrapper for internal iterator Itr.
  template<class Itr>  class iterator_wrapper :
    public std::iterator<
    std::bidirectional_iterator_tag, value_type, std::ptrdiff_t,
    const_pointer, const_reference> {
  public:
    typedef typename Diagnostic::value_type value_type;
    typedef typename Diagnostic::const_pointer pointer;
    typedef typename Diagnostic::const_reference reference;

    iterator_wrapper() = default;

    reference operator*() const { return mCurItr->get(); }
    pointer operator->() const { return &operator*(); }

    bool operator==(const iterator_wrapper &RHS) const {
      return mCurItr == RHS.mCurItr;
    }
    bool operator!=(const iterator_wrapper &RHS) const {
      return mCurItr != RHS.mCurItr;
    }

    iterator_wrapper & operator--() { --mCurItr; return *this; }
    iterator_wrapper & operator++() { ++mCurItr; return *this; }
    iterator_wrapper operator--(int) { auto Tmp = *this; --*this; return Tmp; }
    iterator_wrapper operator++(int) { auto Tmp = *this; ++*this; return Tmp; }

  private:
    friend Diagnostic;
    iterator_wrapper(const Itr &I) : mCurItr(I) {}

    Itr mCurItr;
  };

public:
  typedef typename Collection::size_type size_type;
  typedef iterator_wrapper <Collection::const_iterator> iterator;
  typedef iterator const_iterator;
  typedef iterator_wrapper<Collection::const_reverse_iterator> reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;

  /// Constructs container for diagnostics of a specified kind.
  explicit Diagnostic(const char *Kind) {
    char *Buf = new char[std::strlen(Kind) + 1];
    std::strcpy(Buf, Kind);
    mKind.reset(Buf);
  }

  /// Returns kind of diagnostics.
  const char * getKind() const noexcept { return mKind.get(); }

  /// \brief Returns an iterator to the first element of the container.
  ///
  /// If the container is empty, the returned iterator will be equal to end().
  iterator begin() const { return mDiagnostics.begin(); }

  /// \brief Returns an iterator to the element following the last element of
  /// the container.
  ///
  /// This element acts as a placeholder; attempting to access it results in
  /// undefined behavior.
  iterator end() const { return mDiagnostics.end(); }

  /// \brief Returns an iterator to the first element of the container.
  ///
  /// If the container is empty, the returned iterator will be equal to cend().
  iterator cbegin() const { return mDiagnostics.begin(); }

  /// \brief Returns an iterator to the element following the last element of
  /// the container.
  ///
  /// This element acts as a placeholder; attempting to access it results in
  /// undefined behavior.
  iterator cend() const { return mDiagnostics.end(); }

  /// \brief Returns a reverse iterator to the first element of the reversed
  /// container.
  ///
  /// It corresponds to the last element of the non-reversed container.
  /// If the container is empty, the returned iterator will be equal to rend().
  reverse_iterator rbegin() const { return mDiagnostics.rbegin(); }

  /// \brief Returns a reverse iterator to the element following the last
  /// element of the reversed container.
  ///
  /// It corresponds to the element preceding the first element of the
  /// non-reversed container.
  /// This element acts as a placeholder; attempting to access it results in
  /// undefined behavior.
  reverse_iterator rend() const { return mDiagnostics.rend(); }

  /// \brief Returns a reverse iterator to the first element of the reversed
  /// container.
  ///
  /// It corresponds to the last element of the non-reversed container.
  /// If the container is empty, the returned iterator will be equal to crend().
  reverse_iterator crbegin() const { return mDiagnostics.rbegin(); }

  /// \brief Returns a reverse iterator to the element following the last
  /// element of the reversed container.
  ///
  /// It corresponds to the element preceding the first element of the
  /// non-reversed container.
  /// This element acts as a placeholder; attempting to access it results in
  /// undefined behavior.
  reverse_iterator crend() const { return mDiagnostics.rend(); }

  /// \brief Returns true if the container has no elements.
  ///
  /// This does not consider internal errors (see, internal()).
  bool empty() const { return mDiagnostics.empty(); }

  /// Returns the number of elements in the container.
  ///
  /// This does not consider internal errors (see, internal()).
  size_type size() const { return mDiagnostics.size(); }

  /// Removes all elements from the container including internal errors.
  void clear() { mDiagnostics.clear(); mInternalError = 0; }

  /// Exchanges the contents of the container with those of other
  /// including internal errors.
  void swap(Diagnostic &Other) {
    mDiagnostics.swap(Other.mDiagnostics);
    std::swap(mInternalError, Other.mInternalError);
  }

  /// Returns number of internal erros which have been occurred when
  /// someone try to insert new error in this container.
  size_type internal_size() const noexcept { return mInternalError; }

  /// \brief Inserts new diagnostic to the container.
  ///
  /// \param[in] Code Identifier of a diagnostic.
  /// \param[in] Fmt Pointer to a null-terminated multibyte string specifying
  ///  how to interpret the diagnostic description.
  /// \param[in] Pos Position associated with a diagnostic.
  /// \param[in] A Number of parameters which is going to replace format
  /// placeholders in Fmt string.
  /// \return True if diagnostic has been successfully inserted. In some cases
  /// some internal errors may prevent diagnostic insertion. When this occurs
  /// this method returns false and increments internal errors counter
  /// (see, internal()).
  ///
  /// Each diagnostic is represented as a character string:
  /// <Kind> C<Code>(<Pos>): <Fmt>, Kind may be error, warning, etc.
  template<class... Args>
  bool insert(size_type Code, const char *Fmt, uintmax_t Pos, Args... A) {
    static constexpr const char *PrefixFmt = "%s C%zu(%ju): ";
    auto PreSize = std::snprintf(nullptr, 0, PrefixFmt, mKind.get(), Code, Pos);
    auto ErrSize = std::snprintf(nullptr, 0, Fmt, A...);
    if (PreSize < 0 || ErrSize < 0) {
      ++mInternalError;
      return false;
    }
    auto Size = PreSize + ErrSize + 1;
    char *Buf = new char[Size];
    if (std::snprintf(Buf, PreSize + 1, PrefixFmt, mKind.get(), Code, Pos) < 0 ||
        std::snprintf(Buf + PreSize, ErrSize + 1, Fmt, A...) < 0) {
      ++mInternalError;
      return false;
    }
    mDiagnostics.emplace_back(std::unique_ptr<const char>(Buf));
    return true;
  }
private:
  std::unique_ptr<const char> mKind;
  Collection mDiagnostics;
  size_type mInternalError = 0;
};
}

namespace std {
/// Specializes the std::swap algorithm for bcl::Diagnostic.
inline void swap(bcl::Diagnostic &LHS, bcl::Diagnostic &RHS) {
  LHS.swap(RHS);
}
}
#endif//BCL_DIAGNOSTIC_H

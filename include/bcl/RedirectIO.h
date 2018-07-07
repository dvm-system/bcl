//===--- RedirectIO.h ----- Input/Output Redirection ------------*- C++ -*-===//
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
// This file proposes redirection functionality for input/output operations.
// This is similar to using '>' in command line. For example, to implement
// ./a.out 2>data.err in the code of 'a' the RedirectIO can be used as follows:
//
// // a.cpp
// #include <bcl/RedirectIO.h>
// int main() {
//   bcl::RedirectIO R(STDERR_FILENO, "data.err");
//   // now all writings in stderr will be redirected to "data.err"
// }
//
//===----------------------------------------------------------------------===//

#ifndef BCL_REDIRECT_IO_H
#define BCL_REDIRECT_IO_H

#include "Diagnostic.h"
#include "utility.h"
#include <assert.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

#if defined(_MSC_VER)
# include <io.h>
# define dup2 _dup2
#define fileno _fileno
#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
# define STDERR_FILENO 2
#endif
#elif defined(__CYGWIN__)
# include <io.h>
#else
# include <unistd.h>
#endif

namespace bcl {
// This proposes redirection functionality for input/output operations.
class RedirectIO {
public:
  /// The target file for redirection will be opened according to this mode.
  enum class Mode : char {
    Rewrite = 'w',
    Append = 'a',
    Read = 'r',
  };

  /// \brief Default constructor, nothing will be redirected.
  ///
  /// Note that only move operations are available, copy constructor and
  /// assignment are deleted.
  RedirectIO() : mFile(nullptr) {}

  /// \brief Redirect file with handle descriptor From to a file with a name To.
  ///
  /// The To file will be opened in a mode according to a specified node M.
  /// In case of Mode::Rewrite the file will be opened in mode "w+" and
  /// in case of Mode::Append the file will be opened in mode "a+".
  /// In case of Mode::Read the file will be opened in mode "r+.
  /// In case of errors the file will not be opened and description of errors
  /// becomes available from errors() container.
  RedirectIO(int From, const std::string &To, Mode M = Mode::Rewrite) :
    mErrors(new bcl::Diagnostic("redirect error")),
    mFile(nullptr), mFromHandle(From), mToName(To) {
    const char ModeStr[3] = { static_cast<char>(M), '+', '\0' };
    mFile = std::fopen(To.data(), ModeStr);
    if (!mFile) {
      storeErrNo("fopen");
      return;
    }
    if (dup2(fileno(mFile), mFromHandle) == -1) {
      storeErrNo("dup2");
      if (fclose(mFile) != 0)
        storeErrNo("fclose");
      else
        mFile = nullptr;
      return;
    }
    if (M == Mode::Append) {
      // This is necessary to move position to the end of the file,
      // otherwise a result of ftell() points to the beginning of the file.
      while (fgetc(mFile) != EOF);
      if (!std::feof(mFile)) {
        storeErrNo("fgetc");
        if (fclose(mFile) != 0)
          storeErrNo("fclose");
        else
          mFile = nullptr;
        return;
      }
    }
    mDiffStart = mDiffPos = std::ftell(mFile);
    if (mDiffPos == -1) {
      storeErrNo("ftell");
      if (fclose(mFile) != 0)
        storeErrNo("fclose");
      else
        mFile = nullptr;
      return;
    }
  }

  /// Destructor closes a target file but redirected descriptor stays open and
  /// redirection is still performed.
  ~RedirectIO() {
    // Note that when move operation is executed destructor also called,
    // to determine this case let us use mayRedirected().
    // This is possible because mErrors will be unset when move is performed.
    if (!mayRedirected() || !mFile)
      return;
    if (fclose(mFile) != 0)
      storeErrNo("fclose");
    mFile = nullptr;
  }

  RedirectIO(const RedirectIO &) = delete;
  RedirectIO & operator=(const RedirectIO &) = delete;
  RedirectIO(RedirectIO &&) = default;
  RedirectIO & operator=(RedirectIO &&) = default;

  /// \brief Returns true if redirection is active, note that in case of errors
  /// this also returns true.
  ///
  /// \return This returns false only if default constructor has been used
  /// to create a RedirectIO object.
  bool mayRedirected() const noexcept { return mErrors != nullptr; }

  /// Returns container of errors.
  const bcl::Diagnostic & errors() const noexcept {
    assert(mayRedirected() && "Redirection has not been activated (default constructor was used)!");
    return *mErrors;
  }

  /// Returns true if errors have been occurred, internal errors are
  /// also considered.
  bool hasErrors() const {
    return mayRedirected() &&
      (!errors().empty() || errors().internal_size() > 0);
  }

  /// Returns a name of target file for redirection.
  const std::string & fileName() const noexcept { return mToName; }

  /// \brief Returns true if some changes have been made since the last
  /// position setting.
  ///
  /// \return If some errors occur this method is going to return false and
  /// store errors description in a diagnostic container (see errors()).
  bool isDiff() {
    // It is necessary to perform this check because fflush(NULL) flushes all
    // opened files and this is unwanted behavior.
    if (!mFile) {
      storeErrNo(EINVAL, "fflush");
      return false;
    }
    if (std::fflush(mFile) != 0) {
      storeErrNo("fflush");
      return false;
    }
    auto Pos = std::ftell(mFile);
    if (Pos == -1) {
      storeErrNo("ftell");
      return false;
    }
    return Pos != mDiffPos;
  }

  /// \brief Returns changes that have been made since the last
  /// position setting.
  ///
  /// \return If some errors occur this method is going to return empty string
  /// and store errors description in a diagnostic container (see errors()).
  /// If input is redirected this method returns data which have not been
  /// read yet.
  std::string diff() {
    assert(mFile && "File must not be null!");
    if (!isDiff())
      return "";
    auto Pos = std::ftell(mFile);
    if (Pos == -1) {
      storeErrNo("ftell");
      return "";
    }
    if (std::fseek(mFile, mDiffPos, SEEK_SET) != 0) {
      storeErrNo("fseek");
      return "";
    }
    char Buf[100] = "\0";
    std::string Str;
    while (std::fgets(Buf, 100, mFile))
      Str += Buf;
    if (!std::feof(mFile)) {
      storeErrNo("fgets");
      if (std::fseek(mFile, Pos, SEEK_SET) != 0)
        storeErrNo("fseek");
      return "";
    }
    return Str;
  }

  /// Reset position to perform diff() comparison. Position will the same
  /// as when a redirection has been occurred.
  void resetDiffPos() noexcept { mDiffPos = mDiffStart; }

  /// Set position to perform diff() comparison to a current position.
  bool setDiffPos() {
    assert(mFile && "File must not be null!");
    mDiffPos = std::ftell(mFile);
    if (mDiffPos == -1) {
      storeErrNo("ftell");
      return false;
    }
    return true;
  }

private:
  /// Stores description of error ErrNos to the errors() container.
  ///
  /// \param [in] Number of error. This must be one of possible errno values.
  /// \param [in] Op This is an operation which produces an error.
  void storeErrNo(int ErrNo, const char *Op) {
    auto Error = std::strerror(ErrNo);
    mErrors->insert(ErrNo, "%s: %c%s (%s)",
      0, mToName.data(), std::tolower(Error[0]), Error + 1, Op);
  }

  /// Stores description of error available from errno macros to the errors()
  /// container.
  ///
  /// \param [in] Op This is an operation which produces an error.
  void storeErrNo(const char *Op) {
    auto Error = std::strerror(errno);
    mErrors->insert(errno, "%s: %c%s (%s)",
      0, mToName.data(), std::tolower(Error[0]), Error + 1, Op);
  }

  std::unique_ptr<bcl::Diagnostic> mErrors;
  int mFromHandle;
  std::string mToName;
  FILE *mFile;
  long mDiffPos;
  long mDiffStart;
};
}

#endif//BCL_REDIRECT_IO_H

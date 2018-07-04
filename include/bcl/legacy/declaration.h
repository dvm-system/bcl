//===--- declaration.h ------- Base Declaration -----------------*- C++ -*-===//
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
// This file contains declarations of general data entities.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_DECLARATION_H
#define BCL_DECLARATION_H

#ifndef BCL_LEGACY
#define BCL_LEGACY
#endif//BCL_LEGACY

#include <string>

namespace Utility {
  template<class Type_, class Nullable_> class Value;
}

/// TODO (kaniandr@gmail.com): a legacy code should be removed when it will
/// become possible
#ifdef BCL_LEGACY
//! Содержит базовые объекты программы.
namespace Base {
  struct BCL;
  template< class Application_ > struct Exception;
  typedef Exception<BCL> BCLException; 

  typedef std::wstring TextUnicode; //!< Текстовая строка в формате UNICODE.
  typedef wchar_t CharUnicode; //!< Символ в формате UNICODE.
  inline void CopyUnicode( CharUnicode *to, size_t toSize, const CharUnicode *from);
  inline TextUnicode ToTextUnicode( int value);

  typedef std::string TextAnsi; //!< Текстовая строка в формате ANSI.
  typedef char CharAnsi; //!< Символ в формате ANSI.
  inline void CopyAnsi( CharAnsi *to, size_t toSize, const CharAnsi *from);
  inline TextAnsi ToTextAnsi( int value);

  typedef TextAnsi Text; //!< Текстовая строка в формате ANSI.
  typedef CharAnsi Char; //!< Символ в формате ANSI.
  inline Text ToText( int value);
  inline void CopyString(Char *to, size_t toSize, const Char *from);
# define TEXT(String_) String_ //!< Макрос для представления строки в формате ANSI.
#endif
}

/// Defines a static method with a specified name 'name_' that returns a
/// specified string 'text_'.
#define TO_TEXT(name_, text_) \
  static inline const std::string & name_( ) {\
    static const std::string Text(text_);\
    return Text; \
}

/// \brief Defines description field for a type.
///
/// It can be used as a subclass of some other class which should be described.
#define DESCRIPTION_FIELD(name_, field_, data_) \
  struct name_ {\
    TO_TEXT(Field, field_) \
    TO_TEXT(Data, data_) \
    \
    typedef Utility::Null ValueType; \
  };

/// Defines entity which is not used and avoids unused warnings.
#define UNUSED_ARG( Arg_) ( Arg_)

#ifdef _DEBUG
# ifndef DEBUG 
/// This is defined om debug mode and is similar to _DEBUG macro.
#  define DEBUG _DEBUG
# endif
#endif

#ifdef DEBUG
# ifndef _DEBUG
/// This is defined om debug mode and to DEBUG macro.
#  define _DEBUG DEBUG 
# endif

/// Proposes additional function parameter that is available in debug mode only.
# define DEBUG_PARAM(Expr_) , Expr_

/// Proposes variable declaration that is available in debug mode only. 
# define DEBUG_DECL(Type_, Var_) Type_ Var_

/// Proposes a statement that is available in debug mode only.  
# define DEBUG_STMT(Stmt_ ) Stmt_
#else
/// Proposes additional function parameter that is available in debug mode only.
# define DEBUG_PARAM(Expr_)

/// Proposes variable declaration that is available in debug mode only. 
# define DEBUG_DECL( Type_, Var_)

/// Proposes a statement that is available in debug mode only.
# define DEBUG_STMT( Stmt_ )

# undef NDEBUG 
/// This is defined in release mode.
# define NDEBUG
#endif

/// TODO (kaniandr@gmail.com): a legacy code should be removed when it will
/// become possible
#ifdef BCL_LEGACY
# ifndef ASSERT
#  include <assert.h>
#  define ASSERT( Predct_) assert( Predct_)
# endif
#endif

#endif//BCL_DECLARATION

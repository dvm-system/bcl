/*! \file
	\brief ���������� ���� ������� �������, ����� � ������ �������� ������.		
*/

#ifndef DECLARATION_H
#define DECLARATION_H

/* ���������� ��������������, ��������� ��������� ������������� �������� ���������� � �� ���������.
   ������ ����������� �++ �� �������������� Visual Studio 2008 � �������� ��������������.
*/
#pragma warning( disable : 4290 ) 

#include <string>

/*! \defgroup Text ������������� ��������� �����
	\brief �������� ����������� ��������� ���������, ���������� �� ������������� ��������� ����� � ���������.

	� ����������� �� ������������ ��������� (UNICODE ��� ANSI), ������������ ������������ �������� \c _UNICODE �
	\c UNICODE,	��������, �������� � ������ ������, ����� ���������� ��-�������.
	\warning ��� ���������� ������������� ��������� ���������� ������������ �������������� � ������ ��������,
	� �� �� ���������� �����������.
	@{ 
*/
#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE //!< ���������� ������, ����������� �� ������������� � ��������� ��������� UNICODE.
#endif
#endif

//! �������� ������� ������� ���������.
namespace Base
{
	typedef std::wstring TextUnicode; //!< ��������� ������ � ������� UNICODE.
	typedef wchar_t CharUnicode; //!< ������ � ������� UNICODE.

	typedef std::string TextAnsi; //!< ��������� ������ � ������� ANSI.	
	typedef char CharAnsi; //!< ������ � ������� ANSI.		
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE //!< ���������� ������, ����������� �� ������������� � ��������� ��������� UNICODE.
#endif
	typedef TextUnicode Text; //!< ��������� ������ � ������� UNICODE.
	typedef CharUnicode Char; //!< ������ � ������� UNICODE.	
#define TEXT( String_) L##String_ //!< ������ ��� ������������� ������ � ������� UNICODE.
#else
	typedef TextAnsi Text; //!< ��������� ������ � ������� ANSI.	
	typedef CharAnsi Char; //!< ������ � ������� ANSI.		
#define TEXT( String_) String_ //!< ������ ��� ������������� ������ � ������� ANSI.
#endif
}

//! ���������� ����������� ����� ������ ��� ��������� ��������� ���������� � ������.
/*! \param name_ ��� ������.
	\param text_ ������������ ����������.
*/
#define TO_TEXT( name_, text_) static inline const Base::Text& name_( ) { static const Base::Text text( text_); return text;}
//!@}

//! ���������� �������������� � ��������� ������.
/*! ��������� �������� �������������� ���������� ������������ ������ � ������������� � ��������� �������������� ��������.
*/
#define UNUSED_ARG( Arg_) ( Arg_)

/*! \defgroup Debug �������� ����������� ������
	\brief �������� ��������������� ��������, ������� �������� � ���������� ������,
	�� ������������ ��� ���������� � ������ release. 
	
	���������� ����� ������������ �������� ��� ���������� �������� \c _DEBUG ��� \c DEBUG.
	��� ����������� ���������� ������ ������������� ������������ ������ \c NDEBUG.
	� Microsoft Visual Studio ������ \c _DEBUG ������������ ������������� ��� ������ � ���������� ������.
	//@{
*/
#ifdef _DEBUG
#ifndef DEBUG 
#define DEBUG _DEBUG //!< ���������� ������, ������������ ��� �������� ������ � ���������� ������.
#endif
#endif

#ifdef DEBUG

#ifndef _DEBUG
#define _DEBUG DEBUG //!< ���������� ������, ������������ ��� �������� ������ � ���������� ������.
#endif

#define DEBUG_PARAM( Expr_) , Expr_ //!< ������ �������������� �������� �������, ��������� ������ � ���������� ������.
#define DEBUG_DECL( Type_, Var_) Type_ Var_ //!< ������� ���������� ����������, ��������� ������ � ���������� ������.

#else

#define DEBUG_PARAM( Expr_) //!< ������ �������������� �������� �������, ��������� ������ � ���������� ������.
#define DEBUG_DECL( Type_, Var_) //!< ������� ���������� ����������, ��������� ������ � ���������� ������.
#define NDEBUG //!< ������������ ��� ����������� ���������� ������.

#endif

#ifndef ASSERT
#include <assert.h>
#define ASSERT( Predct_) assert( Predct_) //!< ������ ��� �������� � ���������� ������ ���������� ����������� � ���������.
#endif
//@}

#endif//DB_DECLARATION
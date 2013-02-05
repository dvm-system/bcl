/*! \file
	\brief Объявления всех базовых классов, типов и других структур данных.		
*/

#ifndef DECLARATION_H
#define DECLARATION_H

/* Игнорирует предупреждения, вызванные указанием выбрасываемых функцией исключений в ее заголовке.
   Данная возможность С++ не поддерживается Visual Studio 2008 и вызывает предупреждения.
*/
#pragma warning( disable : 4290 ) 

#include <string>

/*! \defgroup Text Представление текстовых строк
	\brief Содержит определения различных сущностей, отвечающих за представление текстовых строк в программе.

	В зависимости от используемой кодировки (UNICODE или ANSI), определяемой доступностью макросов \c _UNICODE и
	\c UNICODE,	сущности, входящие в данную группу, будут определены по-разному.
	\warning Для сохранения переносимости программы необходимо использовать представленные в группе сущности,
	а не их конкретные определения.
	@{ 
*/
#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE //!< Определяет макрос, указывающие на использование в программе кодировки UNICODE.
#endif
#endif

//! Содержит базовые объекты программы.
namespace Base
{
	typedef std::wstring TextUnicode; //!< Текстовая строка в формате UNICODE.
	typedef wchar_t CharUnicode; //!< Символ в формате UNICODE.

	typedef std::string TextAnsi; //!< Текстовая строка в формате ANSI.	
	typedef char CharAnsi; //!< Символ в формате ANSI.		
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE //!< Определяет макрос, указывающие на использование в программе кодировки UNICODE.
#endif
	typedef TextUnicode Text; //!< Текстовая строка в формате UNICODE.
	typedef CharUnicode Char; //!< Символ в формате UNICODE.	
#define TEXT( String_) L##String_ //!< Макрос для представления строки в формате UNICODE.
#else
	typedef TextAnsi Text; //!< Текстовая строка в формате ANSI.	
	typedef CharAnsi Char; //!< Символ в формате ANSI.		
#define TEXT( String_) String_ //!< Макрос для представления строки в формате ANSI.
#endif
}

//! Определяет статический метод класса для получения текстовой информации о классе.
/*! \param name_ Имя метода.
	\param text_ Возвращаемая информацию.
*/
#define TO_TEXT( name_, text_) static inline const Base::Text& name_( ) { static const Base::Text text( text_); return text;}
//!@}

//! Определяет неиспользуемый в процедуре объект.
/*! Позволяет избежать предупреждений выдаваемых компилятором всвязи с пристутствием в программе неиспользуемых объектов.
*/
#define UNUSED_ARG( Arg_) ( Arg_)

/*! \defgroup Debug Средства отладочного режима
	\brief Содержит вспомогательные сущности, которые доступны в отладочном режиме,
	но игнорируются при компиляции в режиме release. 
	
	Отладочный режим определяется заданием при компиляции макросов \c _DEBUG или \c DEBUG.
	При отключенном отладочном режиме автоматически определяется макрос \c NDEBUG.
	В Microsoft Visual Studio макрос \c _DEBUG определяется автоматически при сборке в отладочном режиме.
	//@{
*/
#ifdef _DEBUG
#ifndef DEBUG 
#define DEBUG _DEBUG //!< Определяет макрос, используемый для проверки сборки в отладочном режиме.
#endif
#endif

#ifdef DEBUG

#ifndef _DEBUG
#define _DEBUG DEBUG //!< Определяет макрос, используемый для проверки сборки в отладочном режиме.
#endif

#define DEBUG_PARAM( Expr_) , Expr_ //!< Задает дополнительный параметр функции, доступный только в отладочном режиме.
#define DEBUG_DECL( Type_, Var_) Type_ Var_ //!< Создает объявление переменной, доступное только в отладочном режиме.

#else

#define DEBUG_PARAM( Expr_) //!< Задает дополнительный параметр функции, доступный только в отладочном режиме.
#define DEBUG_DECL( Type_, Var_) //!< Создает объявление переменной, доступное только в отладочном режиме.
#define NDEBUG //!< Определяется при отключенном отладочном режиме.

#endif

#ifndef ASSERT
#include <assert.h>
#define ASSERT( Predct_) assert( Predct_) //!< Макрос для проверки в отладочном режиме выполнения ограничений в программе.
#endif
//@}

#endif//DB_DECLARATION
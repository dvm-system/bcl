/*! \file
    \brief Содержит конструкции для работы с текстовыми строками.
*/

#ifndef TEXT_H
#define TEXT_H

#include "declaration.h"
#include "exception.h"

#include <stdlib.h>
#include <string>

namespace Base
{
     //! Копирует строку в формате UNICODE
    inline void CopyUnicode( CharUnicode *to, size_t toSize, const CharUnicode *from)
    {
        if ( wcscpy_s( to, toSize, from) != 0)
            throw Base::Exception< BCL >::Error< CELL_COLL_1( ErrorList::CopyString) >( );
    }

    //! Преобразует число в строку в формате UNICODE.
    inline TextUnicode ToTextUnicode( int value)
    {
        CharUnicode buffer[ 15];
        if ( _itow_s( value, buffer, 15, 10) == 0)
            return buffer;

        return L"";
    }

    //! Копирует строку в формате ANSI
    inline void CopyAnsi( CharAnsi *to, size_t toSize, const CharAnsi *from)
    {
        if ( strcpy_s( to, toSize, from) != 0)
            throw Base::Exception< BCL >::Error< CELL_COLL_1( ErrorList::CopyString) >( );
    }

    //! Преобразует число в строку в формате ANSI.
    inline TextAnsi ToTextAnsi( int value)
    {
        CharAnsi buffer[ 15];
        if ( _itoa_s( value, buffer, 15, 10) == 0)
            return buffer;

        return "";
    }

#ifdef UNICODE
    //! Преобразует число в строку в формате UNICODE.
    inline Text ToText( int value) { return ToTextUnicode( value); }
    //! Копирует строки в формате UNICODE.
    inline void CopyString( Char *to, size_t toSize, const Char *from)
    {
        CopyUnicode( to, toSize, from); 
    }
#else
    //! Преобразует число в строку в формате ANSI.
    inline Text ToText( int value) { return ToTextAnsi( value); }
    //! Копирует строки в формате ANSI.
    inline void CopyString( Char *to, size_t toSize, const Char *from)
    {
        CopyAnsi( to, toSize, from); 
    }
#endif
}

#endif//TEXT_H


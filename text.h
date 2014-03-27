/*! \file
    \brief Содержит конструкции для работы с текстовыми строками.
*/

#ifndef TEXT_H
#define TEXT_H

#include "declaration.h"
#include "exception.h"

#include <stdlib.h>
#include <string.h>

namespace Base
{
     //! Копирует строку в формате UNICODE
    inline void CopyUnicode( CharUnicode *to, size_t toSize, const CharUnicode *from)
    {
#ifdef _WIN32
        if ( wcscpy_s( to, toSize, from) != 0)
#else
        if ( wcslen( from) < toSize)
            wcscpy( to, from);
        else
#endif
            throw Base::Exception< BCL >::Error< CELL_COLL_1( ErrorList::CopyString) >( );
    }

    //! Преобразует число в строку в формате UNICODE.
    inline TextUnicode ToTextUnicode( int value)
    {
        CharUnicode buffer[ 15];

#pragma warning( suppress : 4996)
        _itow( value, buffer, 10);
        
        return buffer;
    }

    //! Копирует строку в формате ANSI
    inline void CopyAnsi( CharAnsi *to, size_t toSize, const CharAnsi *from)
    {
#ifdef _WIN32
        if ( strcpy_s( to, toSize, from) != 0)
#else
        if ( strlen( from) < toSize)
            strcpy( to, from);
        else
#endif
            throw Base::Exception< BCL >::Error< CELL_COLL_1( ErrorList::CopyString) >( );
    }

    //! Преобразует число в строку в формате ANSI.
    inline TextAnsi ToTextAnsi( int value)
    {
        CharAnsi buffer[ 15];

#pragma warning( suppress : 4996)
        _itoa( value, buffer, 10);

        return buffer;
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

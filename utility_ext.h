/*! \file
	\brief Содержит дополнения к вспомогательным сущностям.
*/
#ifndef UTILITY_EXT_H
#define UTILITY_EXT_H

#include "utility.h"

//! \todo  убрать _WIN32
#ifdef _WIN32
	#include <memory>
#else
	#include <tr1/memory>
#endif

namespace Utility
{	
	//! Универсальный интерфейс для доступа к методам объектов (см. \ref IObject)
	/*! \ingroup IObject
		Реализация для объектов при доступе к которым используется управляемый указатель \c std::tr1::shared_ptr.
	*/
	template< class Object_ > struct IObject< std::tr1::shared_ptr< Object_ > >
	{
		typedef Object_ Object; //!< Исползуемый объект.
		typedef std::tr1::shared_ptr< Object_ > Internal; //!< Внутреннее представление объекта.
		typedef std::tr1::shared_ptr< Object_ > Reference; //!< Ссылка на объект.
		typedef std::tr1::shared_ptr< const Object_ > ReferenceC; //!< Константная ссылка на объект.

	public:
		//! Выполняет разыменование объекта.
		inline static Object& Dereference( Reference object) { return *object;}
	
	public:
		//! Конструктор умолчания, создает объект типа \c Object в динамической памяти.
		inline IObject( ) : m_object( new Object) { }

		//! Инициализация интерфейса с помощью конструктора копирования.
		/*! Объект предоставляет доступ к памяти, на которую указывает параметр конструктора.
			Другой способ инициализации интерфейса - использование конструктора умолчания.
			\param [in, out] object Объект, указывающий на память к которой предоставляется доступ.
		*/
		inline IObject( const Reference object) : m_object( object) { }		
	    
		//! Выполняет разыменование объекта.
		inline Object & operator*( ) { return *m_object;}

		//! Выполняет разыменование объекта.
		inline const Object & operator*( ) const { return *m_object;}

		//! Выполняет разыменование объекта.
		inline Object * operator->( ) { return m_object.get( );}

		//! Выполняет разыменование объекта.
		inline const Object * operator->( ) const { return m_object.get( );}
	    
		//! Возвращает объект.
		inline Reference Get( ) { return m_object; }
	    
	private:
		Internal m_object; //!< Используемый объект.
	};
}
#endif//UTILITY_EXT_H
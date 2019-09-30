/*! \file
	\brief �������� ���������� � ��������������� ���������.
*/
#ifndef UTILITY_EXT_H
#define UTILITY_EXT_H

#include "utility.h"

//! \todo  ������ _WIN32
#ifdef _WIN32
	#include <memory>
#else
	#include <tr1/memory>
#endif

namespace Utility
{	
	//! ������������� ��������� ��� ������� � ������� �������� (��. \ref IObject)
	/*! \ingroup IObject
		���������� ��� �������� ��� ������� � ������� ������������ ����������� ��������� \c std::tr1::shared_ptr.
	*/
	template< class Object_ > struct IObject< std::tr1::shared_ptr< Object_ > >
	{
		typedef Object_ Object; //!< ����������� ������.
		typedef std::tr1::shared_ptr< Object_ > Internal; //!< ���������� ������������� �������.
		typedef std::tr1::shared_ptr< Object_ > Reference; //!< ������ �� ������.
		typedef std::tr1::shared_ptr< const Object_ > ReferenceC; //!< ����������� ������ �� ������.

	public:
		//! ��������� ������������� �������.
		inline static Object& Dereference( Reference object) { return *object;}
	
	public:
		//! ����������� ���������, ������� ������ ���� \c Object � ������������ ������.
		inline IObject( ) : m_object( new Object) { }

		//! ������������� ���������� � ������� ������������ �����������.
		/*! ������ ������������� ������ � ������, �� ������� ��������� �������� ������������.
			������ ������ ������������� ���������� - ������������� ������������ ���������.
			\param [in, out] object ������, ����������� �� ������ � ������� ��������������� ������.
		*/
		inline IObject( const Reference object) : m_object( object) { }		
	    
		//! ��������� ������������� �������.
		inline Object & operator*( ) { return *m_object;}

		//! ��������� ������������� �������.
		inline const Object & operator*( ) const { return *m_object;}

		//! ��������� ������������� �������.
		inline Object * operator->( ) { return m_object.get( );}

		//! ��������� ������������� �������.
		inline const Object * operator->( ) const { return m_object.get( );}
	    
		//! ���������� ������.
		inline Reference Get( ) { return m_object; }
	    
	private:
		Internal m_object; //!< ������������ ������.
	};
}
#endif//UTILITY_EXT_H
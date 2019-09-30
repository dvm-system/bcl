/*! \file
	\brief �������� ������, ����������� ����������� ��������� �����.
*/
#ifndef CELL_H
#define CELL_H

#include "declaration.h"
#include "utility.h"
#include "cell_macros.h"

namespace Base
{
	//! ���������� ����� ������ � ������ ������������������ �����.
	/*! ����� ������������ ��� ������������ ����������� ���������, ������������ ������������������� �����.
		��� ����������� ������ ����������� �� �������������, ��� ��������� ��������,
		����������� ���������� � ��� ��������������� �������� � ��������� �� ��� ������ � ���������.
		������ ����������� � ������ ��������� � ������� ��������� ������������. 
		������ ������ ����������� �� ��������� �� ��� � ��������� ������.
		��������� ������ ����������� �� �������������� ������ �������� ������� \c Utility::Null.
		\attention ������������� ������ - ��������� ������, ������ ������� ������ ���� ����������:
		- ��� \c ValueType �������� ��� �������� �������� � ������;
		- ��� \c Nullable ����������� �������� \c Utility::True ��� \c Utility::False � �������� ����������� ��� �������������
		�������� � ������ �������������� �������� �������������.
		\tparam CellId_ ������������� ������, ������������ ��� ������� � ���.
		\tparam CellNext_ ��������� ������ � ���������.
		\todo �������� �������� ����, ��� ������� ����������� ������ ���� ���.
		\sa \ref cell_test
	*/
	template< class CellId_, class CellNext_ = Utility::Null > 
	class Cell: public CellNext_
	{ 
	public:
		typedef CellId_ CellId; //!< ������������� ������, ������������ ��� ������� � ���.
		typedef typename CellId::ValueType ValueType; //!< ��� ��������, ��������� � ������.   
		typedef typename CellId::Nullable Nullable; //!< ������� ������������ �������������� ��������.
		typedef CellNext_ CellNext; //!< ��������� ������ � ���������.

	public:
		//! \name Service-������
		//@{					
		//! \copybrief Foreach( Functor_ &)
		/*!	��������������� ������������� ��������� ����������� ������ � ��������� ���� ����� � ���������.			
			\attention � ���������� ����������� ������ ���� ��������� ����� 
			`template< class Cell_ > operator( )( )` (� �������� ������� ��. \c ColumnTextListFunctor).
			\par ��������
			���������� ���������� ��� ������ ������, ���� ������ �� ��������� � ���������,
			�� ����������� ������� � ��������� ��������� ������.
			\todo C������ ��������� � ������������ ������� ������ ���������.
			\todo ������ _WIN32
			\sa \ref cell_foreach_definition "������"
		*/			
		template< class Functor_ > static void ForeachDefinition( Functor_ &functor)
		{
#ifdef _WIN32
			functor.operator( )< Cell >( );
#else
			functor.template operator( )< Cell >( );
#endif
			ForeachDefinition< Functor_ >( functor, Utility::IsIdentical< CellNext, Utility::Null >( ));
		} 		
		//@}				

	public:    
		//! \name Access-������
		//@{
		//! ���������� \c true, ���� ������ �������� �������������� ��������.
		/*! ���� ������ �������� ��������� �������� ��� 
			�� ����� ��������� �������������� �������� (������������ ������������� ��������������),
			������ ����� ���������� \c false.
			\warning ���� ������ �������� �������������� ��������, �� ������ ����������� � ��� ��������
			(����� ������ \c GetValue( ) const) �������� � ��������������� ���������.
			\tparam What_ ������������� ������.
			\par ��������
			������� � ����������� �� ����������� ���� \c Cell::Nullable
			����������� ����� �� ������ ��������� �������������� ��������:
			- \c Utility::True ��������, ��� ������ ����� ��������� �������������� �������� � 
			����� ��������� �������� ����� ����������� �� �������� � ������ ��������������� �������� � ������� �����:			
			\code template< class What_ > bool IsNullValue( Utility::True) const \endcode			
			- \c Utility::False ��������, ��� ������ �� ����� ��������� �������������� �������� � 
			����� ���������� �������� \c false � ������� ������: 
			\code template< class What_ > bool IsNullValue( Utility::False) const \endcode
		*/
		template< class What_ > bool IsNullValue( ) const
		{    
			return IsNullValue< What_ >( typename What_::Nullable( ));
		}
    
		//! ��������� � ������ �������������� ��������.
		/*! \tparam What_ ������������� ������.
			\pre ��������� �������� �������������� �������� ��� ������ ������.
			\todo �������� ���� ��� ����� �������� �������������� ��������
		*/
		template< class What_ > void SetNullValue( )
		{
			typedef Utility::Value< typename What_::ValueType, typename What_::Nullable > WhatValue;

			WhatValue &whatValue = const_cast< WhatValue & >( AccessValue< What_ >( ));
			whatValue.SetNull( );
		}
	    
		//! ���������� ��������, �������� � ������.
		/*! \tparam What_ ������������� ������.
			\pre ������ �� �������� �������������� ��������.					
		*/
		template< class What_ > 
		typename Utility::IObject< typename What_::ValueType >::Reference GetValue( ) 
		{ 
			ASSERT( !IsNullValue< What_ >( ));

			typedef Utility::Value< typename What_::ValueType, typename What_::Nullable > WhatValue; 
			
			WhatValue &whatValue = const_cast< WhatValue & >( AccessValue< What_ >( ));
			return whatValue.GetValue( );
		}    

		//! ���������� ��������, �������� � ������.
		/*! \tparam What_ ������������� ������.
			\pre ������ �� �������� �������������� ��������.					
		*/
		template< class What_ > 
		typename Utility::IObject< typename What_::ValueType >::ReferenceC GetValue( ) const
		{ 
			ASSERT( !IsNullValue< What_ >( ));			
			
			return AccessValue< What_ >( ).GetValue( );
		}    
	    
		//! ��������� �������� � ������.
		/*! \tparam What_ ������������� ������.
			\post ������ �� �������� �������������� ��������.
		*/
		template< class What_ > void SetValue( typename Utility::IObject< typename What_::ValueType >::ReferenceC value)
		{ 
			typedef Utility::Value< typename What_::ValueType, typename What_::Nullable > WhatValue; 
			
			WhatValue &whatValue = const_cast< WhatValue & >( AccessValue< What_ >( ));
			whatValue.SetValue( value);						
		}   
		//@}
    
		//! \name Service-������
		//@{				
		//! �������� �������� ���������� ��� ������ ������ � ���������.
		/*!	��������������� ������������� ��������� ����������� ������ � ��������� ���� ����� � ���������.			
			\attention � ���������� ����������� ������ ���� ��������� ����� 
			`template< class Cell_ > operator( )( Cell_ *cell)`.				
			\par ��������
			���������� ���������� ��� ������ ������, ���� ������ �� ��������� � ���������,
			�� ����������� ������� � ��������� ��������� ������.
			\todo C������ ��������� � ������������ ������� ������ ���������.
			\sa \ref cell_foreach "������"
		*/
		template< class Functor_ > void Foreach( Functor_ &functor)
		{
			functor( this);			
			Foreach< Functor_ >( functor, Utility::IsIdentical< CellNext, Utility::Null >( ));
		} 		
		//@}

	protected:
		//! ���������� ��������, �������� � ������.
		/*! \tparam What_ ������������� ������.			
			\par �������� 
			������ ��������������� ��������������� �� ������ ���������, ���� �� ����� ������� ������ ������.
			�������� ����, ��� ������� ������ ������ ����������� � ������� ������ \c Utility::IsIdentical:
			- e��� ������� ������ ������, �� `Utility::IsIdentical< What_, CellId >( )`
			����� �������� ������ ��������������� �� \c Utility::True � ����� ���������� �������� �� ������ � ������� ������: 
			\code template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::True) const \endcode
			- �����, `Utility::IsIdentical< What_, CellId >( )`  
			����� �������� ������ ��������������� �� \c Utility::False � ����� ����������� ��������� ����� � ��������� c ������� ������: 
			\code template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::False) const \endcode			
		*/
		template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( ) const
		{
			return AccessValue< What_ >( Utility::IsIdentical< What_, CellId >( ));
		}

	private:
		//! ���������� ��������, �������� � ������.		
		template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::True) const
		{
			return m_value;
		}

		//! ��������� � ������������ ��������� ������.
		template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::False) const
		{
			return CellNext::template AccessValue< What_ >( );
		}

		//! ���������, ����� ������ �� ����� ��������� �������������� ��������.
		template< class What_ > bool IsNullValue( Utility::False) const { return false;}

		//! ��������� ��� �������������� �������� ��������, ����� ������ ����� ��������� �������������� ��������.		
		template< class What_ > bool IsNullValue( Utility::True) const { return AccessValue< What_ >( ).IsNull( ); }				
		
		//! ���������� ��� �������� � ��������� ��������� ������ � ���������.		
		/*! ������������ ��� ��������� ��������, ���������� � ������� ���������.
		*/
		template< class Functor_ > void Foreach( Functor_ &functor, Utility::False) 
		{ 
			CellNext::template Foreach< Functor_ >( functor);
		}    

		//! ����������, ����� ���������� ���� ������ � ���������.
		/*! ������������ ��� ��������� ��������, ���������� � ������� ���������.
		*/
		template< class Functor_ > void Foreach( Functor_ &functor, Utility::True) { }    		

		//! ���������� ��� �������� � ��������� ��������� ����� � ���������.
		/*! ������������ ��� ��������� �������� ����� � ���������.
		*/
		template< class Functor_ > static void ForeachDefinition( Functor_ &functor, Utility::False) 
		{ 
			CellNext::template ForeachDefinition< Functor_ >( functor);
		}    

		//! ����������, ����� ���������� ��� ������ � ���������.
		/*! ������������ ��� ��������� �������� ����� � ���������.
		*/
		template< class Functor_ > static void ForeachDefinition( Functor_ &functor, Utility::True) { }    	
    
	private:
		Utility::Value< ValueType, Nullable > m_value; //!< ��������, �������� � ������.   
	};

	//! ���������� ������������ �� �������� ������ � ���������.
	/*! \tparam First_ ������ ������ � ���������.
		\tparam What_ ������������� ������� ������.
		���� ������ � ��������������� `What_` �������� � ��������� 
		��� � ��������� ��� �� ����� ������, 
		�� ������ ����� ����� ����������� �� ������ `Utility::False` �
		� ��� \c Cell ����������� ��������� ������ ����� ��������� ��� \c Utility::Null, �����
		����� ����� ����������� �� `Utility::True` � 
		��� \c Cell ����� ��������� � ����� ��������� ������.		
	*/
	template< class First_, class What_ > struct IsCellExist;

	//! �������� ������ ����������.
	namespace Detail
	{
		//! ��������� ����� ����� � ���������.
		/*! \par ��������
			��������������� ��������������� ��� ������ � ���������, 
			���� �� ����� ������� ������ ��� ���� �� ����� ��������� ����� ���������.
			\tparam Current_ ������ ��������������� � ������ ������.
			\tparam What_ ������������� ������� ������.
			\tparam Bool_ ������� ����, �������� �� �������� ������ �������.			 
		*/
		template< class Current_, class What_, class Bool_ > struct IsCellExistImp;
	
		//! ������������� ��� ������, ����� �������� ������ �������� �������.
		template< class Current_, class What_ > struct IsCellExistImp < Current_, What_, Utility::True > : 
			public Utility::True 
		{ 
			typedef Current_ Cell; //!< ��������� ������.
		};
		
		//! ������������� ��� ������, ����� ������ ��� �� �������.
		template< class Current_, class What_ > struct IsCellExistImp < Current_, What_, Utility::False > : 
			public IsCellExist< typename Current_::CellNext, What_ > { };
	}

	//! ���������� ������, ������������� ������������ �� �������� ������ � ���������.
	template< class First_, class What_ > struct IsCellExist : 
		public Detail::IsCellExistImp< First_, What_,
									   typename Utility::IsIdentical< typename First_::CellId, What_ >::Definition > { };

	//! ������������� ��� ������, ����� ��������� ����� ���������.
	template< class What_ > struct IsCellExist< Utility::Null, What_ > :
		public Utility::False
	{ 
		typedef Utility::Null Cell; //!< ������ �� ���� �������.
	};
}

/*! \page cell_test	������ ������������� ������ Base::Cell. ������ � �������� 
	���������� ������� ������ � �������� ����:
	��� | ��������
	--- | --------
	����| 300.50
	��� ������ ������ ������ ���� ������������� ����������� ������.
	� �������� ����� �������� ���������� � ���� ��������� ����� \c Base::Cell � ���������������� \c Name � \c Salary.
	�������� ��� ������� �������� �� ����� \a \b cell_test.cpp:	
	
	\dontinclude cell_test.cpp
	1. ���������� ����������� ����� � �������������:
	\until iostream
	2. ��������� ������ � ������������ ���� \c Base:
	\skipline using
	3. ��������� �������������� �����, �������� ��������� �������� ����������:
	\n ��� ��������:		
	\skip Name
	\until ;
	��� ������ ������ ���� ������:
	\skipline Nullable
	������� ����������� ����� ���������� �������� ������:
	\until };
	�������� ��������:		
	\skip Salary		
	\until ;
	���� ������� ����� �� ��������, �� ��� �������� �� ����������:
	\until };
	4. \anchor cell_def ��������� ��������� ������, ��������� �� ���� �����:
	\skip CELL
	\line CELL
	\line CELL
	5. \anchor cell_foreach_definition
	������� ��������� ��� ������ ���������� � ��������� ������ 
	� ������� ������������ ������ \c Base::Cell::ForeachDefinition( ):
	\skip Functor
	\until };
	6. \anchor cell_foreach
	������� ���������� ��� ������ ���������� � �������� \c Base::Cell::Foreach( ):
	\skip Functor
	\until };
	7. ������� ������ ������, ��������� �� ���������� � ��������.
	��� ������ ���� ������ �� ������� ��� ������ (������������ ������������� ������), 
	����� ��� ������ ��������� ����� �����������:
	\skip main
	\until }
	� ���������� ������� ��������� �������:		
	\n `Structure of salary information collection:`
	\n `Name is <value>`
	\n `Salary is <value>`
	\n `Worker's salary before he starts wroking:`
	\n `Name is Smit`
	\n `Salary is NULL`
	\n `Worker's salary after he starts wroking:`
	\n `Name is Smit`
	\n `Salary is 300.5`
*/

#endif//�ELL_H

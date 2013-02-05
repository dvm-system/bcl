/*! \file
	\brief Содержит классы, реализующие статические коллекции ячеек.
*/
#ifndef CELL_H
#define CELL_H

#include "declaration.h"
#include "utility.h"
#include "cell_macros.h"

namespace Base
{
	//! Определяет новую ячейку в начале последовательности ячеек.
	/*! Класс предназначен для статического определения коллекций, образованных последовательностью ячеек.
		При определении ячейки указывается ее идентификатор, тип хранимого значения,
		возможность сохранения в ней неопределенного значения и следующая за ней ячейка в коллекции.
		Ячейка добавляется в начало коллекции с помощью механизма наследования. 
		Каждая ячейка наследуется от следующей за ней в коллекции ячейки.
		Последняя ячейка наследуется от неопределенной ячейки заданной классом \c Utility::Null.
		\attention Идентификатор ячейки - структура данных, внутри которой должны быть определены:
		- тип \c ValueType задающий тип значений хранимых в ячейке;
		- тип \c Nullable принимающий значения \c Utility::True или \c Utility::False и задающий возможность или невозможность
		хранения в ячейке неопределенных значения соответсвенно.
		\tparam CellId_ Идентификатор ячейки, используется для доступа к ней.
		\tparam CellNext_ Следующая ячейка в коллекции.
		\todo Добавить проверку того, что столбец встречается только один раз.
		\sa \ref cell_test
	*/
	template< class CellId_, class CellNext_ = Utility::Null > 
	class Cell: public CellNext_
	{ 
	public:
		typedef CellId_ CellId; //!< Идентификатор ячейки, используется для доступа к ней.
		typedef typename CellId::ValueType ValueType; //!< Тип значения, хранимого в ячейке.   
		typedef typename CellId::Nullable Nullable; //!< Признак допустимости неопределнного значения.
		typedef CellNext_ CellNext; //!< Следующая ячейка в коллекции.

	public:
		//! \name Service-методы
		//@{					
		//! \copybrief Foreach( Functor_ &)
		/*!	Последовательно предоставляет заданному функционалу доступ к описаниям всех ячеек в коллекции.			
			\attention В вызываемом функционале должен быть определен метод 
			`template< class Cell_ > operator( )( )` (в качестве примера см. \c ColumnTextListFunctor).
			\par Алгоритм
			Фунцкионал вызывается для данной ячейки, если ячейка не последняя в коллекции,
			то выполняется переход к обработке следующей ячейки.
			\todo Cделать реалзацию с возможностью задания списка функторов.
			\todo Убрать _WIN32
			\sa \ref cell_foreach_definition "Пример"
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
		//! \name Access-методы
		//@{
		//! Возвращает \c true, если ячейка содержит неопределенное значение.
		/*! Если ячейка содержит некоторое значение или 
			не может содержать неопределенное значение (определяется спецификацией идентификатора),
			данный метод возвращает \c false.
			\warning Если ячейка содержит неопределенное значение, то запрос хранящегося в ней значения
			(вызов метода \c GetValue( ) const) приведет к неопределенному поведению.
			\tparam What_ Идентификатор ячейки.
			\par Алгоритм
			Сначала в зависимости от определения типа \c Cell::Nullable
			проверяется может ли ячейка содержать неопределенное значение:
			- \c Utility::True означает, что ячейка может содержать неопределенное значение и 
			будет выполнена проверка флага отвечающего за хранение в ячейке неопределенного значения с помощью метод:			
			\code template< class What_ > bool IsNullValue( Utility::True) const \endcode			
			- \c Utility::False означает, что ячейка не может содержать неопределенное значение и 
			будет возвращено значение \c false с помощью метода: 
			\code template< class What_ > bool IsNullValue( Utility::False) const \endcode
		*/
		template< class What_ > bool IsNullValue( ) const
		{    
			return IsNullValue< What_ >( typename What_::Nullable( ));
		}
    
		//! Сохраняет в ячейке неопределенное значение.
		/*! \tparam What_ Идентификатор ячейки.
			\pre Допустимо задавать неопределенное значение для данной ячейки.
			\todo Проверка того что можно задавать неопределенное значение
		*/
		template< class What_ > void SetNullValue( )
		{
			typedef Utility::Value< typename What_::ValueType, typename What_::Nullable > WhatValue;

			WhatValue &whatValue = const_cast< WhatValue & >( AccessValue< What_ >( ));
			whatValue.SetNull( );
		}
	    
		//! Возвращает значение, хранимое в ячейке.
		/*! \tparam What_ Идентификатор ячейки.
			\pre Ячейка не содержит неопределенное значение.					
		*/
		template< class What_ > 
		typename Utility::IObject< typename What_::ValueType >::Reference GetValue( ) 
		{ 
			ASSERT( !IsNullValue< What_ >( ));

			typedef Utility::Value< typename What_::ValueType, typename What_::Nullable > WhatValue; 
			
			WhatValue &whatValue = const_cast< WhatValue & >( AccessValue< What_ >( ));
			return whatValue.GetValue( );
		}    

		//! Возвращает значение, хранимое в ячейке.
		/*! \tparam What_ Идентификатор ячейки.
			\pre Ячейка не содержит неопределенное значение.					
		*/
		template< class What_ > 
		typename Utility::IObject< typename What_::ValueType >::ReferenceC GetValue( ) const
		{ 
			ASSERT( !IsNullValue< What_ >( ));			
			
			return AccessValue< What_ >( ).GetValue( );
		}    
	    
		//! Сохраняет значение в ячейке.
		/*! \tparam What_ Идентификатор ячейки.
			\post Ячейка не содержит неопределенное значение.
		*/
		template< class What_ > void SetValue( typename Utility::IObject< typename What_::ValueType >::ReferenceC value)
		{ 
			typedef Utility::Value< typename What_::ValueType, typename What_::Nullable > WhatValue; 
			
			WhatValue &whatValue = const_cast< WhatValue & >( AccessValue< What_ >( ));
			whatValue.SetValue( value);						
		}   
		//@}
    
		//! \name Service-методы
		//@{				
		//! Вызывает заданный функционал для каждой ячейки в коллекции.
		/*!	Последовательно предоставляет заданному функционалу доступ к значениям всех ячеек в коллекции.			
			\attention В вызываемом функционале должен быть определен метод 
			`template< class Cell_ > operator( )( Cell_ *cell)`.				
			\par Алгоритм
			Фунцкионал вызывается для данной ячейки, если ячейка не последняя в коллекции,
			то выполняется переход к обработке следующей ячейки.
			\todo Cделать реалзацию с возможностью задания списка функторов.
			\sa \ref cell_foreach "Пример"
		*/
		template< class Functor_ > void Foreach( Functor_ &functor)
		{
			functor( this);			
			Foreach< Functor_ >( functor, Utility::IsIdentical< CellNext, Utility::Null >( ));
		} 		
		//@}

	protected:
		//! Возвращает значение, хранимое в ячейке.
		/*! \tparam What_ Идентификатор ячейки.			
			\par Алгоритм 
			Ячейки рассматриваются последовательно от начала коллекции, пока не будет найдена нужная ячейка.
			Проверка того, что найдена нужная ячейка выполняется с помощью класса \c Utility::IsIdentical:
			- eсли найдена нужная ячейка, то `Utility::IsIdentical< What_, CellId >( )`
			будет объектом класса унаследованного от \c Utility::True и будет возвращено значение из ячейки с помощью метода: 
			\code template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::True) const \endcode
			- иначе, `Utility::IsIdentical< What_, CellId >( )`  
			будет объектом класса унаследованного от \c Utility::False и будет рассмотрена следующая ячека в коллекции c помощью метода: 
			\code template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::False) const \endcode			
		*/
		template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( ) const
		{
			return AccessValue< What_ >( Utility::IsIdentical< What_, CellId >( ));
		}

	private:
		//! Возвращает значение, хранимое в ячейке.		
		template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::True) const
		{
			return m_value;
		}

		//! Переходит к рассмотрению следующей ячейки.
		template< class What_ > const Utility::Value< typename What_::ValueType, typename What_::Nullable >& AccessValue( Utility::False) const
		{
			return CellNext::template AccessValue< What_ >( );
		}

		//! Вызвается, когда ячейка не может содержать неопределенное значение.
		template< class What_ > bool IsNullValue( Utility::False) const { return false;}

		//! Вызвается для дополнительной проверки значения, когда ячейка может содержать неопределенное значение.		
		template< class What_ > bool IsNullValue( Utility::True) const { return AccessValue< What_ >( ).IsNull( ); }				
		
		//! Вызывается при переходе к обработке следующей ячейки в коллекции.		
		/*! Используется при обработке значений, хранящихся в ячейках коллекции.
		*/
		template< class Functor_ > void Foreach( Functor_ &functor, Utility::False) 
		{ 
			CellNext::template Foreach< Functor_ >( functor);
		}    

		//! Вызывается, когда обработаны всей ячейки в коллекции.
		/*! Используется при обработке значений, хранящихся в ячейках коллекции.
		*/
		template< class Functor_ > void Foreach( Functor_ &functor, Utility::True) { }    		

		//! Вызывается при переходе к обработке следующей ячеки в коллекции.
		/*! Используется при обработке описаний ячеек в коллекции.
		*/
		template< class Functor_ > static void ForeachDefinition( Functor_ &functor, Utility::False) 
		{ 
			CellNext::template ForeachDefinition< Functor_ >( functor);
		}    

		//! Вызывается, когда обработаны все ячейки в коллекции.
		/*! Используется при обработке описаний ячеек в коллекции.
		*/
		template< class Functor_ > static void ForeachDefinition( Functor_ &functor, Utility::True) { }    	
    
	private:
		Utility::Value< ValueType, Nullable > m_value; //!< Значение, хранимое в ячейке.   
	};

	//! Определяет присутствует ли заданная ячейка в коллекции.
	/*! \tparam First_ Первая ячейка в коллекции.
		\tparam What_ Идентификатор искомой ячейки.
		Если ячейка с идентификатором `What_` отствует в коллекции 
		или в коллекции нет ни одной ячейки, 
		то данный класс будет унаследован от класса `Utility::False` и
		в тип \c Cell описывающий найденную ячйеку будет определен как \c Utility::Null, иначе
		класс будет унаследован от `Utility::True` и 
		тип \c Cell будет совпадать с типом найденной ячейки.		
	*/
	template< class First_, class What_ > struct IsCellExist;

	//! Содержит детали реализации.
	namespace Detail
	{
		//! Реализует поиск ячеек в коллекции.
		/*! \par Алгоритм
			Последовательно рассматриваются все ячейки в коллекции, 
			пока не будет найдена нужная или пока не будет достигнут конец коллекции.
			\tparam Current_ Ячейка рассматриваемая в данный момент.
			\tparam What_ Идентификатор искомой ячейки.
			\tparam Bool_ Признак того, является ли текующая ячейка искомой.			 
		*/
		template< class Current_, class What_, class Bool_ > struct IsCellExistImp;
	
		//! Специализация для случая, когда текующая ячейка является искомой.
		template< class Current_, class What_ > struct IsCellExistImp < Current_, What_, Utility::True > : 
			public Utility::True 
		{ 
			typedef Current_ Cell; //!< Найденная ячейка.
		};
		
		//! Специализация для случая, когда ячейка еще не найдена.
		template< class Current_, class What_ > struct IsCellExistImp < Current_, What_, Utility::False > : 
			public IsCellExist< typename Current_::CellNext, What_ > { };
	}

	//! Реализация класса, определяющего присутствует ли заданная ячейка в коллекции.
	template< class First_, class What_ > struct IsCellExist : 
		public Detail::IsCellExistImp< First_, What_,
									   typename Utility::IsIdentical< typename First_::CellId, What_ >::Definition > { };

	//! Специализация для случая, когда достигнут конец коллекции.
	template< class What_ > struct IsCellExist< Utility::Null, What_ > :
		public Utility::False
	{ 
		typedef Utility::Null Cell; //!< Ячейка не была найдена.
	};
}

/*! \page cell_test	Пример использования класса Base::Cell. Данные о зарплате 
	Необходимо создать реестр с записями вида:
	Имя | Зарплата
	--- | --------
	Смит| 300.50
	Для каждой записи должна быть предусмотрена возможность печати.
	О человеке будем хрантить информацию в виде коллекции ячеек \c Base::Cell с идентификаторами \c Name и \c Salary.
	Исходный код примера доступен из файла \a \b cell_test.cpp:	
	
	\dontinclude cell_test.cpp
	1. Подключаем необходимые файлы с определениями:
	\until iostream
	2. Разрешаем доступ к пространству имен \c Base:
	\skipline using
	3. Описываем идентификаторы ячеек, задающие структуру хранимой информации:
	\n Имя человека:		
	\skip Name
	\until ;
	Имя всегда должно быть задано:
	\skipline Nullable
	Создаем статический метод печатающий название ячейки:
	\until };
	Зарплата человека:		
	\skip Salary		
	\until ;
	Если человек нигде не работает, то его зарплата не определена:
	\until };
	4. \anchor cell_def Описываем структуру записи, сосотящей из двух ячеек:
	\skip CELL
	\line CELL
	\line CELL
	5. \anchor cell_foreach_definition
	Создаем фнукионал для печати информации о стркутуре записи 
	с помощью статического метода \c Base::Cell::ForeachDefinition( ):
	\skip Functor
	\until };
	6. \anchor cell_foreach
	Создаем функционал для печати информации о человеке \c Base::Cell::Foreach( ):
	\skip Functor
	\until };
	7. Создаем пустую запись, заполняем ее значениями и печатаем.
	Имя должно быть задано до попытки его чтения (определяется спецификацией ячейки), 
	иначе при печати результат будет неопределен:
	\skip main
	\until }
	В результате запуска программы получим:		
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

#endif//СELL_H

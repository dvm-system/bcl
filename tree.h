/*! \file
    \brief Содержит ститическое дерево.
*/
#ifndef STATIC_TREE_H
#define STATIC_TREE_H

#include "utility.h"

namespace Base
{
    template< class Left_ > class LeftBranchImp : public Left_     
    { 
    protected:
        LeftBranchImp< Left_ >( ) { }

        LeftBranchImp< Left_ >( const Left_ &left) : Left_( left) { }
    };

    template< class Right_ > class RightBranchImp : public Right_ 
    { 
    protected:
        RightBranchImp< Right_ >( ) { }

        RightBranchImp< Right_ >( const Right_ &right) : Right_( right) { }
    };

    template< class Id_, class Left_ = Utility::Null, class Right_ = Utility::Null >
    class Node : 
        public LeftBranchImp< Left_ >, 
        public RightBranchImp< Right_ >, 
        public virtual Utility::If< Utility::IsReference< typename Id_::ValueType >,
                                    Utility::Unassignable,
                                    Utility::Copyable >::Result
    {
    public:
        typedef Id_ Id;
        typedef typename Id::ValueType ValueType;

        typedef Left_ Left;
        typedef LeftBranchImp< Left > LeftBranch;

        typedef Right_ Right;        
        typedef RightBranchImp< Right > RightBranch;

        typedef Node< Id, Left, Right > Root; 

    public:
        Node< Id, Left, Right>( ) { }

        Node< Id, Left, Right>( typename Utility::If< Utility::IsReference< ValueType >, ValueType, const ValueType & >::Result value, const Left &left, const Right &right)
            : m_value( value), LeftBranch( left), RightBranch( right) { }

        template< class Query_ >
        Node< Id, Left, Right>( const Query_ &query)
            : m_value( query.template Value< typename Query_::Id >( )), 
            LeftBranch( query.LeftNode( )), 
            RightBranch( query.RightNode( )) { }

        Left & LeftNode( ) { return static_cast< Left & >( static_cast< LeftBranch & >( *this)); }

        const Left & LeftNode( ) const { return static_cast< const Left & >( static_cast< const LeftBranch & >( *this)); }

        Right & RightNode( ) { return static_cast< Right & >( static_cast< RightBranch & >( *this)); }

        const Right & RightNode( ) const { return static_cast< const Right & >( static_cast< const RightBranch & >( *this)); }

        //! \name Access-методы
		//@{
        //! Предоставляет доступ к значению, хранимому в ячейке.
		/*! \tparam What_ Идентификатор ячейки.			
		*/
		template< class What_ > 
        typename What_::ValueType & Value( )  {	return typename IsNodeExist< Root, What_ >::Cell::m_value; }   

        //! Возвращает значение, хранимое в ячейке.
		/*! \tparam What_ Идентификатор ячейки.			
		*/
		template< class What_ > 
        const typename What_::ValueType & Value( ) const { return typename IsNodeExist< Root, What_ >::Cell::m_value; }    				

        //! Предоставляет доступ к значению, хранимому в ячейке.
		/*! \tparam What_ Идентификатор ячейки.			
		*/
		template< class What_ > 
        typename What_::ValueType & operator[ ]( What_) { return Value< What_ >( ); }    

        //! Возвращает значение, хранимое в ячейке.
		/*! \tparam What_ Идентификатор ячейки.			
		*/
		template< class What_ > 
        const typename What_::ValueType & operator[ ]( What_) const { return Value< What_ >( ); }		
		//@}
    
    protected:
        ValueType m_value; //!< Значение хранимое в ячейке.
    };

    template< class First_ > struct CurrentNode
        : public Utility::True
    {
        typedef First_ Cell;
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
	template< class First_, class What_ > struct IsNodeExist
        : public Utility::If< Utility::IsIdentical< typename First_::Id, What_ >,
                              CurrentNode< First_ >, 
                              typename Utility::If< IsNodeExist< typename First_::Left, What_>,
                                                    IsNodeExist< typename First_::Left, What_>,
                                                    IsNodeExist< typename First_::Right, What_> >::Result >::Result { };

    template< class What_ > struct IsNodeExist< Utility::Null, What_ >
        : public Utility::False
	{ 
		typedef Utility::Null Cell; //!< Ячейка не была найдена.
	};	

    template< class Tree_, class Functor_ >
    struct Visitor
    {
        void Visit( Tree_ &tree, Functor_ &functor)
        {
            Visit( static_cast< typename Tree_::Left & >( *this), functor);
            Visit( static_cast< typename Tree_::Right & >( *this), functor);
            functor( this);            
        }
    };
    
}

#endif//STATIC_TREE_H

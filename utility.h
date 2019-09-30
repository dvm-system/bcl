/*! \file
	\brief �������� ��������������� ��������.
*/
#ifndef UTILITY_H
#define UTILITY_H

//! �������� ��������������� ��������.
namespace Utility
{
	//! ��������� ��������� ����.
	/*! \tparam Type_ ��� �������� ���������.
		\tparam value_ �������� ���������.
	*/
	template< class Type_, Type_ value_ > struct Constant
	{
		typedef Constant< Type_, value_ > Definition; //!< �������� ���������.
		typedef Type_ Type; //!< ��� �������� ���������.

		//! ���������� �������� ���������.
		inline static Type Value( ) { return value_; }
	};

	//! �������� ��������.
	typedef Constant< bool, true > True;	

	//! ������ ��������.
	typedef Constant< bool, false > False;

	//! �������������� ��������.
	struct Null { };

	//! �������� ������ ����������.
	namespace Detail
	{
		//! ���������� ��������� ���������� �������� �������������� �������� ������� \c True ��� \c False.
		/*! \tparam Bool_ ���������� ��������, �������������� ������� \c True ��� \c False.
			\note �������� ������ ������ NotImp, ��� ���������� ���������� ��������, �����
			�������� ���������� ��������� ��� ������������� ������������ � ������ \c Not.
			���� ����������� ����� \c Not �� ������ ����, �� �������� ������������ ����������,
			��������� ��������� ����������.
		*/
		template< class Bool_ > struct NotImp;

		//! ���������� ���������: ������������� � ������ �������� ��������� ��������.
		template< > struct NotImp< True > : public False { };

		//! ���������� ���������: ������������� � ������ �������� ������� ��������.
		template< > struct NotImp< False > : public True { };

		//! ���������� ����������� '�'  ���������� �������� �������������� �������� ������� \c True ��� \c False.
		/*! \tparam FirstOp_ ������ ��������, �������������� ������� \c True ��� \c False.
			\tparam SecondOp_ ������ ��������, �������������� ������� \c True ��� \c False.
		*/
		template< class FirstOp_, class SecondOp_ > struct AndImp;

		//! ���������� ����������� '�': ������������� � ������ ���� �������� ��������.
		template< > struct AndImp< True, True > : public True { };

		//! ���������� ����������� '�': ������������� � ������ ��������� � ������� ��������.
		template< > struct AndImp< True, False > : public False { };

		//! ���������� ����������� '�': ������������� � ������ ������� ������� ��������.
		template< class Bool_ > struct AndImp< False, Bool_ > : public False { };		

		//! ���������� ����������� '���'  ���������� �������� �������������� �������� ������� \c True ��� \c False.
		/*! \tparam FirstOp_ ������ ��������, �������������� ������� \c True ��� \c False.
			\tparam SecondOp_ ������ ��������, �������������� ������� \c True ��� \c False.
		*/
		template< class FirstOp_, class SecondOp_ > struct OrImp;

		//! ���������� ����������� '���': ������������� � ������ ���� ������ ��������.
		template< > struct OrImp< False, False > : public False { };

		//! ���������� ����������� '���': ������������� � ������ ������� � ���������� ��������.
		template< > struct OrImp< False, True > : public True { };

		//! ���������� ����������� '���': ������������� � ������ ������� ��������� ��������.
		template< class Bool_ > struct OrImp< True, Bool_ > : public True { };	

		//! ���������� ��������� ���������.
		/*! \tparam Condition_ ������� ���������.
			\tparam TrueBranch_ �������� ����� ���������.
			\tparam FalseBranch_ ������ ����� ���������.
			\todo �������� �������� ����, ��� \c Condition_ - ���������� �����.
		*/
		template< class Condition_, class TrueBranch_, class FalseBranch_ > struct IfImp;

		//! ���������� ��������� ��������� ��� ��������� ������� ���������.
		template< class TrueBranch_, class FalseBranch_ > struct IfImp< True, TrueBranch_, FalseBranch_ > : public TrueBranch_
		{
			typedef TrueBranch_ Result; //!< ��������� ����� ���������.
		};

		//! ���������� ��������� ��������� ��� ������� ������� ���������.
		template< class TrueBranch_, class FalseBranch_ > struct IfImp< False, TrueBranch_, FalseBranch_ > : public FalseBranch_
		{
			typedef FalseBranch_ Result; //!< ��������� ����� ���������.
		};
	}

	//! ��������� ���������.
	/*! \tparam Bool_ ��������� ��������, ����� �������������� �� ����� �� ���������� ��������.
		\todo �������� �������� ����, ��� \c Bool_ - ���������� �����.
		\todo �������� �������� ����, ��� \� Bool_::Value - ���������� ��������� � ������ NotImp.
			  ���������� ��� And.
	*/
	template< class Bool_ > struct Not: public Detail::NotImp< typename Bool_::Definition > { };	

	//! ��������� ���������� '�'.
	/*! \tparam FirstOp_ ������ ��������, ����� �������������� �� ����� �� ���������� ��������.
		\tparam SecondOp_ ������ ��������, ����� �������������� �� ����� �� ���������� ��������.
		\todo �������� �������� ����, ��� \c FirstOp_ � \c SecondOp_ - ���������� ������.
	*/
	template< class FirstOp_, class SecondOp_ > struct And:
		public Detail::AndImp< typename FirstOp_::Definition, typename SecondOp_::Definition > { };

	//! ��������� ���������� '���'.
	/*! \tparam FirstOp_ ������ ��������, ����� �������������� �� ����� �� ���������� ��������.
		\tparam SecondOp_ ������ ��������, ����� �������������� �� ����� �� ���������� ��������.
		\todo �������� �������� ����, ��� \c FirstOp_ � \c SecondOp_ - ���������� ������.
	*/
	template< class FirstOp_, class SecondOp_ > struct Or:
		public Detail::OrImp< typename FirstOp_::Definition, typename SecondOp_::Definition > { };

    //! ��������� �������� ���������.
	/*! ��������� ����� ��������� ����� ���������� � ������� ����������� ���� Result,
		����� ��������� ��������� \c If ����� ����������� �� ��������� ����� ���������.
		\tparam Condition_ ������� ���������.
		\tparam TrueBranch_ �������� ����� ���������.
		\tparam FalseBranch_ ������ ����� ���������.
		\todo �������� �������� ����, ��� \c Condition_ - ���������� �����.
	*/
	template< class Condition_, class TrueBranch_, class FalseBranch_ > struct If:
		public Detail::IfImp< typename Condition_::Definition, TrueBranch_, FalseBranch_ > { };

	//! ��������� �������� ����������� �����.
	template< class What_, class Whis_ > struct IsIdentical: public False { };
	
	//! ������������� ���������� ������, ��� ������ ���������� �����.
	template< class What_ > struct IsIdentical< What_, What_ > : public True { };

	//! ��������� �������� ����, ��� ��� �������� ����������.
	template< class What_ > struct IsPointer : public False { };

	//! ������������� ���������� ������, ����� ��� �������� ����������.
	template< class What_ > struct IsPointer< What_ * > : public True{ };
	
	//! ������������ ������, ����� � ��������� ������� �� ����� ��������� ������ ������.
	/*! \tparam Type_ ��� �������������� �������.
		\pre ��� �������� ���� Type_ ������ ���� ���������� �������� '+='.
	*/
	template< class Type_ > inline void AddToObjectIf( Utility::False, Type_ &, const Type_ &) { }
	
	//! ��������� ������ � ��������� �������.
	/*! \tparam Type_ ��� �������������� �������.
		\param [in,out] object ������, � �������� ����������� ������ \c what.
		\param [in] what ����������� ������.
		\pre ��� �������� ���� Type_ ������ ���� ���������� �������� '+='.
	*/
	template< class Type_ > inline void AddToObjectIf( Utility::True, Type_ &object, const Type_ &what) { object += what;}

	//! ��������� �������� �����������, �������� ������ ��������.
	/*! \tparam Object_ ������ �� ��������� ������.
		\tparam Classificator_ �������������, ������������ ������ ����������� ��������.
		\tparam Class_ ������, � ������� ��������� ��������������� ������.	
		\todo �������� ������ �������������.
	*/
	template< class Object_, template< class Object_ > class Classificator_, class Class_ > struct GroupTraits;

	/*! \defgroup IObject IObject-������
		\brief ������������� ��������� ��� ������� � ������� ��������.
		
		�� �������� �� ����, ���������� �� ������ ������ � ������������ ������ ���
		����������� ������, ������������ �� ����������� ���������, ������ ��������� ��������� 
		���������� � ������� ������� ��������� �������: <tt> obj->Method(...) </tt>.	
		 \tparam Object_ ������������ ������.
		 \attention � ������ Object_ ������ ���� ���������� ����������� ��������� ��� �����������.
		@{
	*/	
	//! ������������� ��������� ��� ������� � ������� �������� (��. \ref IObject)
	/*!	���������� ��� ��������, ������������� � ����������� ������.		
	*/
	template< class Object_ > struct IObject
	{
		typedef Object_ Object; //!< ����������� ������.
		typedef Object_ Internal; //!< ���������� ������������� �������.
		typedef Object_ & Reference; //!< ������ �� ������.
		typedef const Object_ & ReferenceC; //!< ����������� ������ �� ������.
	 
	public:
		//! ��������� ������������� �������.
		inline static Object & Dereference( Reference object) { return object;}

	public:	
		//! ����������� ���������.
		inline IObject( ) {}

		//! ������������� ���������� � ������� ������������ �����������.
		/*! ������ ������ ������������� ���������� - ������������� ������������ ���������.
			\param [in] object ������ � ����� �������� ��������������� ������.
			\attention ��� ������� � ������ ����� ������������ ����� ��������� �������,
			� �� ��� ������.
		*/
		inline IObject( Reference object) : m_object( object) { }
		
		//! ��������� ������������� �������.
		inline Object& operator* ( ) { return m_object; }

		//! ��������� ������������� �������.
		inline const Object& operator* ( ) const { return m_object; }

		//! ��������� ������������� �������.
		inline Object * operator->( ) { return &m_object;}

		//! ��������� ������������� �������.
		inline const Object * operator->( ) const { return &m_object;}
	    
		//! ���������� ������.
		inline Reference Get( ) { return m_object; }

	private:
		Internal m_object; //!< ������������ ������.
	};

	//! ������������� ��������� ��� ������� � ������� �������� (��. \ref IObject)	
	/*! ���������� ��� ��������, ������������� � ����������� ������ � ������������ � ������� ������� �� � (* ��� ->).		
	*/
	template< class Object_ > struct IObject< Object_ * >
	{
		typedef Object_ Object; //!< ����������� ������.
		typedef Object_ * Internal; //!< ���������� ������������� �������.
		typedef Object_ * Reference; //!< ������ �� ������.
		typedef const Object_ * ReferenceC; //!< ����������� �� ������ ������.

	public:
		//! ��������� ������������� �������.
		inline static Object & Dereference( Reference object) { return *object;}
	
	public:
		//! ����������� ���������, ������� ������ ���� \c Object � ������������ ������.
		inline IObject( ) : m_object( new Object) { } 

		//! ������������� ���������� � ������� ������������ �����������.
		/*! ������ ������������� ������ � ������, �� ������� ��������� �������� ������������.
			������ ������ ������������� ���������� - ������������� ������������ ���������.
			\param [in, out] object ������, ����������� �� ������ � ������� ��������������� ������.
		*/
		inline IObject( Reference object) : m_object( object) { } 		
	    
		//! ��������� ������������� �������.
		inline Object & operator*( ) { return *m_object;}

		//! ��������� ������������� �������.
		inline const Object & operator*( ) const { return *m_object;}

		//! ��������� ������������� �������.
		inline Object * operator->( ) { return m_object;}

		//! ��������� ������������� �������.
		inline const Object * operator->( ) const { return m_object;}
	    
		//! ���������� ������.
		inline Reference Get( ) { return m_object; }
	    
	private:
		Internal m_object; //!< ������������ ������.
	};

	//! ������������� ��������� ��� ������� � ������� �������� (��. \ref IObject)	
	/*! ���������� ��� ������ �� �������.		
	*/
	template< class Object_ > struct IObject< Object_ & >
	{
		typedef Object_ Object; //!< ����������� ������.
		typedef Object_ & Internal; //!< ���������� ������������� �������.
		typedef Object_ & Reference; //!< ������ �� ������.
		typedef const Object_ & ReferenceC; //!< ����������� �� ������ ������.

	public:
		//! ��������� ������������� �������.
		inline static Object & Dereference( Reference object) { return object;}
	
	public:
		//! ������������� ���������� � ������� ������������ �����������.
		/*! ������ ������������� ������ � ������, �� ������� ��������� �������� ������������.
			\warning ������������� ������������ ��������� ��� ������ � ��������� ��������� �� �����������.
			\param [in, out] object ������, ����������� �� ������ � ������� ��������������� ������.
		*/
		inline IObject( Reference object) : m_object( object) { } 		
	    
		//! ��������� ������������� �������.
		inline Object & operator*( ) { return m_object;}

		//! ��������� ������������� �������.
		inline const Object & operator*( ) const { return m_object;}

		//! ��������� ������������� �������.
		inline Object * operator->( ) { return &m_object;}

		//! ��������� ������������� �������.
		inline const Object * operator->( ) const { return &m_object;}
	    
		//! ���������� ������.
		inline Reference Get( ) { return m_object; }
	    
	private:
		Internal m_object; //!< ������������ ������.
	};

	//@}

	/*! \defgroup Value-������
		\brief ������������� ��������� ��� �������� �������� (�������� ��������������) ��������� ����.

		��������� ��������� �������� ��������, ��������� �������� �� �������� ���������������
		(��������� ������ ��� �������, ����������� ������� ��������������� ��������),
		��������� ������������ ������� ��������������� ��������. ���������� ���������� ��������������
		� ����������� �� ���� ��������� ��������.
		@{
	*/	
	//! �������� (�������� �������������) ��������� ����.
	/*! \tparam Type_ ��� ��������� ��������.
		\tparam Nullable_ ������� ������������ (Utility::True ��� Utility::False) �������������� ��������.
	*/
	template< class Type_, class Nullable_ > struct Value;	
	
	//! ������������� ������ Value ��� �������� �������, � ������ �������������� �������������� ��������.
	/*! \tparam Type_ ��� ��������� ��������.
		\tparam Nullable_ ������� ������������ �������������� ��������, �������� �������� Utility::False.
	*/
	template< class Type_ > struct Value< Type_, Utility::False > 
	{	
		typedef Type_ Type; //!< ��� ��������� ��������.
		typedef Utility::False Nullable; //!< ������� ������������ �������������� ��������.			

		//! ���������� ��������.	
		/*! \pre �������� ���� ���������.
		*/
		inline typename Utility::IObject< Type >::Reference GetValue( ) { return m_value; }	

		//! ���������� ��������.
		/*! \pre �������� ���� ���������.
		*/
		inline typename Utility::IObject< Type >::ReferenceC GetValue( ) const { return m_value; }	

		//! ���������� \c false, ��� ��� �������� ������ ����������.
		inline bool IsNull( ) const { return false; }
		
		//! ������ ����� ��������.
		inline void SetValue( typename Utility::IObject< Type >::ReferenceC value) { m_value = value; }

	private:
		typename Utility::IObject< Type >::Internal m_value; //!< �������� ��������.	
	};

	//! ������������� ������ Value ��� �������� �������, � ������ ������������ �������������� ��������.
	/*! \tparam Type_ ��� ��������� ��������.
		\tparam Nullable_ ������� ������������ �������������� ��������, �������� �������� Utility::True.
	*/
	template< class Type_ > struct Value< Type_, Utility::True >
	{ 
		typedef Type_ Type; //!< ��� ��������� ��������.
		typedef Utility::True Nullable; //!< ������� ������������ �������������� ��������.			

		//! ����������� ���������.
		inline explicit Value( ) : m_isNull( true) { } 
		
		//! ���������� ��������.	
		/*! \pre �������� ���� ���������.
		*/
		inline typename Utility::IObject< Type >::Reference GetValue( ) { return m_value; }	

		//! ���������� ��������.
		/*! \pre �������� ���� ���������.
		*/
		inline typename Utility::IObject< Type >::ReferenceC GetValue( ) const { return m_value; }	

		//! ���������� \c true, ���� �������� ��������������.
		inline bool IsNull( ) const { return m_isNull; }

		//! ������ ����� ��������.
		inline void SetValue( typename Utility::IObject< Type >::ReferenceC value) 
		{ 
			m_value = value;
			m_isNull = false;
		}

		//! ������������� �������������� ��������.
		inline void SetNull( ) { m_isNull = true; }

	private:			
		bool m_isNull; //!< ������� ���������������� �������.
		typename Utility::IObject< Type >::Internal m_value; //!< �������� ��������.	
	};

	//! ������������� ������ Value ��� �������� ����������, � ������ ������������ �������������� ��������.
	/*! \tparam Type_ ��� ��������� ��������.	
		\tparam Nullable_ ������� ������������ �������������� ��������, �������� �������� Utility::True.
	*/
	template< class Type_ > struct Value< Type_ *, Utility::True >
	{ 
		typedef Type_ * Type; //!< ��� ��������� ��������.
		typedef Utility::True Nullable; //!< ������� ������������ �������������� ��������.			

		//! ����������� ���������.
		inline explicit Value( ) : m_value( NULL) { }

		//! ���������� ��������.
		/*! \pre �������� ���� ���������.
		*/
		inline typename Utility::IObject< Type >::Reference GetValue( ) { return m_value; }	

		//! ���������� ��������.
		/*! \pre �������� ���� ���������.
		*/
		inline typename Utility::IObject< Type >::ReferenceC GetValue( ) const { return m_value; }	

		//! ���������� \c true, ���� �������� ��������������.
		inline bool IsNull( ) const { return GetValue( ) == NULL; }

		//! ������ ����� ��������.
		/*! \todo ������ const_cast
		*/
		inline void SetValue( typename Utility::IObject< Type >::ReferenceC value) 
		{ m_value = const_cast< typename Utility::IObject< Type >::Reference >( value); }

		//! ������������� �������������� ��������.
		inline void SetNull( ) { SetValue( NULL); }	

	private:
		typename Utility::IObject< Type >::Internal m_value; //!< �������� ��������.
	};										
	//@}
}

#endif//UTILITY_H

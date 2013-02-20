/*! \file
	\brief Содержит вспомогательные сущности.
*/
#ifndef UTILITY_H
#define UTILITY_H

//! Содержит вспомогательные сущности.
namespace Utility
{
	//! Константа заданного типа.
	/*! \tparam Type_ Тип значения константы.
		\tparam value_ Значение константы.
	*/
	template< class Type_, Type_ value_ > struct Constant
	{
		typedef Constant< Type_, value_ > Definition; //!< Описание константы.
		typedef Type_ Type; //!< Тип значения константы.

		//! Возвращает значение константы.
		inline static Type Value( ) { return value_; }
	};

	//! Истинное значение.
	typedef Constant< bool, true > True;	

	//! Ложное значение.
	typedef Constant< bool, false > False;

	//! Неопределенное значения.
	struct Null { };

	//! Содержит детали реализации.
	namespace Detail
	{
		//! Реализация отрицания логических констант представленных классами классом \c True или \c False.
		/*! \tparam Bool_ Отрицаемое значение, представляемое классом \c True или \c False.
			\note Отдельно создан класса NotImp, для реализации логических констант, чтобы
			избежать повторного отрицания при использовании наследования в классе \c Not.
			Если наследовать класс \c Not от самого себя, то возможна некорректная реализация,
			вызванная повторным отрицанием.
		*/
		template< class Bool_ > struct NotImp;

		//! Реализация отрицания: специализация в случае орицания истинного значения.
		template< > struct NotImp< True > : public False { };

		//! Реализация отрицания: специализация в случае орицания ложного значения.
		template< > struct NotImp< False > : public True { };

		//! Реализация логического 'и'  логических констант представленных классами классом \c True или \c False.
		/*! \tparam FirstOp_ Первый аргумент, представленный классом \c True или \c False.
			\tparam SecondOp_ Второй аргумент, представленный классом \c True или \c False.
		*/
		template< class FirstOp_, class SecondOp_ > struct AndImp;

		//! Реализация логического 'и': специализация в случае двух истинных значений.
		template< > struct AndImp< True, True > : public True { };

		//! Реализация логического 'и': специализация в случае истинного и ложного значений.
		template< > struct AndImp< True, False > : public False { };

		//! Реализация логического 'и': специализация в случае первого ложного значения.
		template< class Bool_ > struct AndImp< False, Bool_ > : public False { };		

		//! Реализация логического 'или'  логических констант представленных классами классом \c True или \c False.
		/*! \tparam FirstOp_ Первый аргумент, представленный классом \c True или \c False.
			\tparam SecondOp_ Второй аргумент, представленный классом \c True или \c False.
		*/
		template< class FirstOp_, class SecondOp_ > struct OrImp;

		//! Реализация логического 'или': специализация в случае двух ложных значений.
		template< > struct OrImp< False, False > : public False { };

		//! Реализация логического 'или': специализация в случае ложного и иситинного значений.
		template< > struct OrImp< False, True > : public True { };

		//! Реализация логического 'или': специализация в случае первого истинного значения.
		template< class Bool_ > struct OrImp< True, Bool_ > : public True { };	

		//! Реализация оператора ветвления.
		/*! \tparam Condition_ Условие ветвления.
			\tparam TrueBranch_ Истинная ветвь ветвления.
			\tparam FalseBranch_ Ложная ветвь ветвления.
			\todo Добавить проверку того, что \c Condition_ - логический класс.
		*/
		template< class Condition_, class TrueBranch_, class FalseBranch_ > struct IfImp;

		//! Реализация оператора ветвления для истинного условия ветвления.
		template< class TrueBranch_, class FalseBranch_ > struct IfImp< True, TrueBranch_, FalseBranch_ > : public TrueBranch_
		{
			typedef TrueBranch_ Result; //!< Выбранная ветвь ветвления.
		};

		//! Реализация оператора ветвления для ложного условия ветвления.
		template< class TrueBranch_, class FalseBranch_ > struct IfImp< False, TrueBranch_, FalseBranch_ > : public FalseBranch_
		{
			typedef FalseBranch_ Result; //!< Выбранная ветвь ветвления.
		};
	}

	//! Выполняет отрицание.
	/*! \tparam Bool_ Орицаемое значение, класс унаследованный от одной из логических констант.
		\todo Добавить проверку того, что \c Bool_ - логический класс.
		\todo Добавить проверку того, что \с Bool_::Value - логическая константа и убрать NotImp.
			  Аналогично для And.
	*/
	template< class Bool_ > struct Not: public Detail::NotImp< typename Bool_::Definition > { };	

	//! Выполняет логическое 'и'.
	/*! \tparam FirstOp_ Первый аргумент, класс унаследованный от одной из логических констант.
		\tparam SecondOp_ Второй аргумент, класс унаследованный от одной из логических констант.
		\todo Добавить проверку того, что \c FirstOp_ и \c SecondOp_ - логические классы.
	*/
	template< class FirstOp_, class SecondOp_ > struct And:
		public Detail::AndImp< typename FirstOp_::Definition, typename SecondOp_::Definition > { };

	//! Выполняет логическое 'или'.
	/*! \tparam FirstOp_ Первый аргумент, класс унаследованный от одной из логических констант.
		\tparam SecondOp_ Второй аргумент, класс унаследованный от одной из логических констант.
		\todo Добавить проверку того, что \c FirstOp_ и \c SecondOp_ - логические классы.
	*/
	template< class FirstOp_, class SecondOp_ > struct Or:
		public Detail::OrImp< typename FirstOp_::Definition, typename SecondOp_::Definition > { };

    //! Выполняет оператор ветвления.
	/*! Выбранная ветвь ветвления будет определена с помощью внутреннего типа Result,
		класс оператора ветвления \c If будет унаследован от выбранной ветви ветвления.
		\tparam Condition_ Условие ветвления.
		\tparam TrueBranch_ Истинная ветвь ветвления.
		\tparam FalseBranch_ Ложная ветвь ветвления.
		\todo Добавить проверку того, что \c Condition_ - логический класс.
	*/
	template< class Condition_, class TrueBranch_, class FalseBranch_ > struct If:
		public Detail::IfImp< typename Condition_::Definition, TrueBranch_, FalseBranch_ > { };

	//! Выполняет проверку совпадаения типов.
	template< class What_, class Whis_ > struct IsIdentical: public False { };
	
	//! Специализация шаблонного класса, для случае совпадения типов.
	template< class What_ > struct IsIdentical< What_, What_ > : public True { };

	//! Выполняет проверку того, что тип является указателем.
	template< class What_ > struct IsPointer : public False { };

	//! Специализация шаблонного класса, когда тип является указателем.
	template< class What_ > struct IsPointer< What_ * > : public True{ };
	
	//! Обрабатывает случай, когда к заданному объекту не нужно добавлять другой объект.
	/*! \tparam Type_ Тип обрабатываемых обектов.
		\pre Для объектов типа Type_ должна быть определена операция '+='.
	*/
	template< class Type_ > inline void AddToObjectIf( Utility::False, Type_ &, const Type_ &) { }
	
	//! Добавляет объект в заданному объекту.
	/*! \tparam Type_ Тип обрабатываемых обектов.
		\param [in,out] object Объект, к которому добавляется объект \c what.
		\param [in] what Добавляемый объект.
		\pre Для объектов типа Type_ должна быть определена операция '+='.
	*/
	template< class Type_ > inline void AddToObjectIf( Utility::True, Type_ &object, const Type_ &what) { object += what;}

	//! Позволяет выделить особенности, присущие группе объектов.
	/*! \tparam Object_ Объект из некоторой группы.
		\tparam Classificator_ Классификатор, определяющий способ группировки объектов.
		\tparam Class_ Группа, к которой относится рассматриваемый объект.	
		\todo Добавить пример использования.
	*/
	template< class Object_, template< class Object_ > class Classificator_, class Class_ > struct GroupTraits;

	/*! \defgroup IObject IObject-классы
		\brief Универсальный интерфейс для доступа к методам объектов.
		
		Не зависимо от того, расположен ли данный объект в динамической памяти или
		статической памяти, используются ли управляемый указатели, данный интерфейс позволяет 
		обращаться к методам объекта следующим образом: <tt> obj->Method(...) </tt>.	
		 \tparam Object_ Используемый объект.
		 \attention В классе Object_ должен быть реализован конструктор умолчания или копирования.
		@{
	*/	
	//! Универсальный интерфейс для доступа к методам объектов (см. \ref IObject)
	/*!	Реализация для объектов, расположенных в статической памяти.		
	*/
	template< class Object_ > struct IObject
	{
		typedef Object_ Object; //!< Исползуемый объект.
		typedef Object_ Internal; //!< Внутреннее представление объекта.
		typedef Object_ & Reference; //!< Ссылка на объект.
		typedef const Object_ & ReferenceC; //!< Константная ссылка на объект.
	 
	public:
		//! Выполняет разыменование объекта.
		inline static Object & Dereference( Reference object) { return object;}

	public:	
		//! Конструктор умолчания.
		inline IObject( ) {}

		//! Инициализация интерфейса с помощью конструктора копирования.
		/*! Другой способ инициализации интерфейса - использование конструктора умолчания.
			\param [in] object Объект к копии которого предоставляется доступ.
			\attention При доступе к данным будет использована копия исходного объекта,
			а не сам объект.
		*/
		inline IObject( Reference object) : m_object( object) { }
		
		//! Выполняет разыменование объекта.
		inline Object& operator* ( ) { return m_object; }

		//! Выполняет разыменование объекта.
		inline const Object& operator* ( ) const { return m_object; }

		//! Выполняет разыменование объекта.
		inline Object * operator->( ) { return &m_object;}

		//! Выполняет разыменование объекта.
		inline const Object * operator->( ) const { return &m_object;}
	    
		//! Возвращает объект.
		inline Reference Get( ) { return m_object; }

	private:
		Internal m_object; //!< Используемый объект.
	};

	//! Универсальный интерфейс для доступа к методам объектов (см. \ref IObject)	
	/*! Реализация для объектов, расположенных в динаической памяти и используемых с помощью средств ЯП С (* или ->).		
	*/
	template< class Object_ > struct IObject< Object_ * >
	{
		typedef Object_ Object; //!< Исползуемый объект.
		typedef Object_ * Internal; //!< Внутреннее представление объекта.
		typedef Object_ * Reference; //!< Ссылка на объект.
		typedef const Object_ * ReferenceC; //!< Константная на ссылка объект.

	public:
		//! Выполняет разыменование объекта.
		inline static Object & Dereference( Reference object) { return *object;}
	
	public:
		//! Конструктор умолчания, создает объект типа \c Object в динамической памяти.
		inline IObject( ) : m_object( new Object) { } 

		//! Инициализация интерфейса с помощью конструктора копирования.
		/*! Объект предоставляет доступ к памяти, на которую указывает параметр конструктора.
			Другой способ инициализации интерфейса - использование конструктора умолчания.
			\param [in, out] object Объект, указывающий на память к которой предоставляется доступ.
		*/
		inline IObject( Reference object) : m_object( object) { } 		
	    
		//! Выполняет разыменование объекта.
		inline Object & operator*( ) { return *m_object;}

		//! Выполняет разыменование объекта.
		inline const Object & operator*( ) const { return *m_object;}

		//! Выполняет разыменование объекта.
		inline Object * operator->( ) { return m_object;}

		//! Выполняет разыменование объекта.
		inline const Object * operator->( ) const { return m_object;}
	    
		//! Возвращает объект.
		inline Reference Get( ) { return m_object; }
	    
	private:
		Internal m_object; //!< Используемый объект.
	};

	//! Универсальный интерфейс для доступа к методам объектов (см. \ref IObject)	
	/*! Реализация для ссылок на объекты.		
	*/
	template< class Object_ > struct IObject< Object_ & >
	{
		typedef Object_ Object; //!< Исползуемый объект.
		typedef Object_ & Internal; //!< Внутреннее представление объекта.
		typedef Object_ & Reference; //!< Ссылка на объект.
		typedef const Object_ & ReferenceC; //!< Константная на ссылка объект.

	public:
		//! Выполняет разыменование объекта.
		inline static Object & Dereference( Reference object) { return object;}
	
	public:
		//! Инициализация интерфейса с помощью конструктора копирования.
		/*! Объект предоставляет доступ к памяти, на которую указывает параметр конструктора.
			\warning Использование конструктора умолчания при работе с сылочными объектами не допускается.
			\param [in, out] object Объект, указывающий на память к которой предоставляется доступ.
		*/
		inline IObject( Reference object) : m_object( object) { } 		
	    
		//! Выполняет разыменование объекта.
		inline Object & operator*( ) { return m_object;}

		//! Выполняет разыменование объекта.
		inline const Object & operator*( ) const { return m_object;}

		//! Выполняет разыменование объекта.
		inline Object * operator->( ) { return &m_object;}

		//! Выполняет разыменование объекта.
		inline const Object * operator->( ) const { return &m_object;}
	    
		//! Возвращает объект.
		inline Reference Get( ) { return m_object; }
	    
	private:
		Internal m_object; //!< Используемый объект.
	};

	//@}

	/*! \defgroup Value-классы
		\brief Универсальный интерфейс для хранения значений (возможно неопределенных) заданного типа.

		Интерфейс позволяет задавать значения, проверять являются ли значения неопределенными
		(допустимо только для величин, допускающих задание неопределенного значения),
		управлять возможностью задания неопределенного значения. Реализация интерфейса оптимизирована
		в зависиомсти от типа хранимого значения.
		@{
	*/	
	//! Значение (возможно неопределнное) заданного типа.
	/*! \tparam Type_ Тип хранимого значения.
		\tparam Nullable_ Признак допустимости (Utility::True или Utility::False) неопределнного значения.
	*/
	template< class Type_, class Nullable_ > struct Value;	
	
	//! Специализация класса Value для хранения величин, в случае недопустимости неопределенных значений.
	/*! \tparam Type_ Тип хранимого значения.
		\tparam Nullable_ Признак допустимости неопределнного значения, принимет значение Utility::False.
	*/
	template< class Type_ > struct Value< Type_, Utility::False > 
	{	
		typedef Type_ Type; //!< Тип хранимого значения.
		typedef Utility::False Nullable; //!< Признак допустимости неопределнного значения.			

		//! Возвращает значение.	
		/*! \pre Значение было присвоено.
		*/
		inline typename Utility::IObject< Type >::Reference GetValue( ) { return m_value; }	

		//! Возвращает значение.
		/*! \pre Значение было присвоено.
		*/
		inline typename Utility::IObject< Type >::ReferenceC GetValue( ) const { return m_value; }	

		//! Возвращает \c false, так как значение всегда определено.
		inline bool IsNull( ) const { return false; }
		
		//! Задает новое значение.
		inline void SetValue( typename Utility::IObject< Type >::ReferenceC value) { m_value = value; }

	private:
		typename Utility::IObject< Type >::Internal m_value; //!< Хранимое значение.	
	};

	//! Специализация класса Value для хранения величин, в случае допустимости неопределенных значений.
	/*! \tparam Type_ Тип хранимого значения.
		\tparam Nullable_ Признак допустимости неопределнного значения, принимет значение Utility::True.
	*/
	template< class Type_ > struct Value< Type_, Utility::True >
	{ 
		typedef Type_ Type; //!< Тип хранимого значения.
		typedef Utility::True Nullable; //!< Признак допустимости неопределнного значения.			

		//! Конструктор умолчания.
		inline explicit Value( ) : m_isNull( true) { } 
		
		//! Возвращает значение.	
		/*! \pre Значение было присвоено.
		*/
		inline typename Utility::IObject< Type >::Reference GetValue( ) { return m_value; }	

		//! Возвращает значение.
		/*! \pre Значение было присвоено.
		*/
		inline typename Utility::IObject< Type >::ReferenceC GetValue( ) const { return m_value; }	

		//! Возвращает \c true, если значение неопределенное.
		inline bool IsNull( ) const { return m_isNull; }

		//! Задает новое значение.
		inline void SetValue( typename Utility::IObject< Type >::ReferenceC value) 
		{ 
			m_value = value;
			m_isNull = false;
		}

		//! Устанавливает неопределенное значение.
		inline void SetNull( ) { m_isNull = true; }

	private:			
		bool m_isNull; //!< Признак неопределенности значеня.
		typename Utility::IObject< Type >::Internal m_value; //!< Хранимое значение.	
	};

	//! Специализация класса Value для хранения указателей, в случае допустимости неопределенных значений.
	/*! \tparam Type_ Тип хранимого значения.	
		\tparam Nullable_ Признак допустимости неопределнного значения, принимет значение Utility::True.
	*/
	template< class Type_ > struct Value< Type_ *, Utility::True >
	{ 
		typedef Type_ * Type; //!< Тип хранимого значения.
		typedef Utility::True Nullable; //!< Признак допустимости неопределнного значения.			

		//! Конструктор умолчания.
		inline explicit Value( ) : m_value( NULL) { }

		//! Возвращает значение.
		/*! \pre Значение было присвоено.
		*/
		inline typename Utility::IObject< Type >::Reference GetValue( ) { return m_value; }	

		//! Возвращает значение.
		/*! \pre Значение было присвоено.
		*/
		inline typename Utility::IObject< Type >::ReferenceC GetValue( ) const { return m_value; }	

		//! Возвращает \c true, если значение неопределенное.
		inline bool IsNull( ) const { return GetValue( ) == NULL; }

		//! Задает новое значение.
		/*! \todo Убрать const_cast
		*/
		inline void SetValue( typename Utility::IObject< Type >::ReferenceC value) 
		{ m_value = const_cast< typename Utility::IObject< Type >::Reference >( value); }

		//! Устанавливает неопределенное значение.
		inline void SetNull( ) { SetValue( NULL); }	

	private:
		typename Utility::IObject< Type >::Internal m_value; //!< Хранимое значение.
	};										
	//@}
}

#endif//UTILITY_H

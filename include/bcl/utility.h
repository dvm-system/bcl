//===----- utility.h ---------- Auxiliary Entities --------------*- C++ -*-===//
//
//                       Base Construction Library (BCL)
//
// Copyright 2018 Nikita Kataev
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//
//
// This file defines auxiliary classes and functions that could be useful in
// distinct cases.
//
//===----------------------------------------------------------------------===//

#ifndef BCL_UTILITY_H
#define BCL_UTILITY_H

#include <climits>
#include <cstring>
#include <type_traits>
#include <memory>
#include <string>
#include <utility>

#ifndef BCL_ALWAYS_INLINE
# ifndef _WIN32
#  define BCL_ALWAYS_INLINE __attribute__((always_inline))
# else
#  ifdef __ICL // set for Intel Compiler
#   define BCL_ALWAYS_INLINE __attribute__((always_inline))
#  else
#   define BCL_ALWAYS_INLINE
#  endif
# endif
#endif

/// \brief This can be used to set list of parameters as single parameter
/// of a macro.
///
/// Usage: SOME_MACRO(BCL_JOIN(std::map<int, int>)).
/// In this case ',' does not split std::map<...> into two separate parameters,
/// SOME_MACRO(std::map<int, int>) will not work.
# define BCL_JOIN(...) __VA_ARGS__

namespace bcl {
/// \brief Type of an attribute identifier.
///
/// Each attribute has a distinct identifier which is set in
/// the declaration of the attribute.
typedef const char * AttributeId;

/// \brief Declaration of the attribute with a specified name
/// and type of values.
///
/// Use static method id() to access a distinct attribute identifier.
#define BASE_ATTR_DEF(name_, type_) \
  class name_ { \
  public: \
    typedef type_ Value; \
    static bcl::AttributeId id() { \
      static const char mId = 0; \
      return &mId; \
    } \
};

namespace detail {
/// Implementation of a user-defined binary-literal.
template <char... Bits> struct ToBinary;

/// Implementation of a user-defined binary-literal.
template <char HighBit, char... Bits>
struct ToBinary<HighBit, Bits...> {
  static_assert(HighBit == '0' || HighBit == '1', "Not a binary value!");
  static const unsigned long long Value =
    (HighBit - '0') << (sizeof...(Bits)) | ToBinary<Bits...>::Value;
};

/// Implementation of a user-defined binary-literal.
template <char HighBit>
struct ToBinary<HighBit> {
  static_assert(HighBit == '0' || HighBit == '1', "Not a binary value!");
  static const unsigned long long Value = (HighBit - '0');
};
}

/// \brief Implementation of a user-defined binary-literal.
///
/// Binary-literal is not supported yet in C++ 11 standard (only C++ 14)
/// so this user-defined-literal can be used instead, usage: 01000_b.
/// To use this add `using bcl::operator "" _b;`
template <char... Bits>
constexpr unsigned long long operator "" _b() {
  return detail::ToBinary<Bits...>::Value;
}

/// Returns a number of less significant digits equal to zero in a value which
/// is represented in a number system with the given base.
template<class T>
inline constexpr std::size_t numberOfLessZeros(T Value, T Base) {
  static_assert(std::is_integral<T>::value, "T must be an integral type!");
  return Value % Base != 0 ? 0 : numberOfLessZeros(Value / Base, Base) + 1;
}

/// Returns a number of digits in a value which is represented in a number
/// system with the given base.
template<class T>
inline constexpr std::size_t numberOfDigits(T Value, T Base) {
  static_assert(std::is_integral<T>::value, "T must be an integral type!");
  return Value / Base == 0 ? 1 : numberOfDigits(Value / Base, Base) + 1;
}

/// Returns a unit mask (11...1) of a specified type that comprises a specified
/// number of digits.
template<class T>
inline constexpr T unitMask(std::size_t Digits) {
  static_assert(std::is_integral<T>::value, "T must be an integral type!");
  return Digits == 0 ? static_cast<T>(0) :
    static_cast<T>(1) << (Digits - 1) | unitMask<T>(Digits - 1);
}

/// Prints the bit representation of a specified value to a specified stream.
template<class T, class Stream> void  bitPrint(T V, Stream &OS) {
  for (int BitIdx = sizeof(T) * CHAR_BIT - 1; BitIdx >= 0; --BitIdx)
    OS << ((V >> BitIdx) & 1);
}

/// This prevents assignment of the derived classes.
class Unassignable {
protected:
  constexpr Unassignable() = default;
  ~Unassignable() = default;

  Unassignable(const Unassignable &) = default;
  Unassignable(Unassignable &&) = default;

  Unassignable &operator=(Unassignable &&) = default;
public:
  Unassignable & operator=(const Unassignable &) = delete;
};

/// This prevents copying of the derived classes.
class Uncopyable {
protected:
  constexpr Uncopyable() = default;
  ~Uncopyable() = default;

  Uncopyable(Uncopyable &&) = default;
  Uncopyable &operator=(Uncopyable &&) = default;
public:
  Uncopyable(const Uncopyable &) = delete;
  Uncopyable & operator=(const Uncopyable &) = delete;
};

/// \brief Checks if Ty is contained in the list of types Args.
///
/// If Ty is contained in Args provides the member constant `value` equal to true.
/// Otherwise `value` is false.
template<class Ty, class... Args> struct is_contained;

/// Checks if Ty is contained in the list of types Args.
template<class Ty> struct is_contained<Ty> : public std::false_type {};

/// Checks if Ty is contained in the list of types Args.
template<class Ty, class First, class... Args>
struct is_contained<Ty, First, Args...> :
  public std::conditional<std::is_same<Ty, First>::value,
  std::true_type, is_contained<Ty, Args...>>::type{};

namespace detail {
template<class Ty, class... Args> struct IndexOfImp;

template<class Ty, class Arg>
struct IndexOfImp<Ty, Arg> {
  static constexpr std::size_t index_of() {
    return 0;
  }
};

template<class Ty, class First, class... Args>
struct IndexOfImp<Ty, First, Args...> {
  static constexpr std::size_t index_of() {
    return std::is_same<Ty, First>::value ? 0 :
      IndexOfImp<Ty, Args...>::index_of() + 1;
  }
};

template<class... Args> struct SizeOfImp;

template<> struct SizeOfImp<> {
  static constexpr std::size_t size_of() { return 0; }
};

template<class First, class... Args>
struct SizeOfImp<First, Args...> {
  static constexpr std::size_t size_of() {
    return SizeOfImp<Args...>::size_of() + 1;
  }
};
}

/// Returns index of type Ty in the list of types Args.
template<class Ty, class... Args> inline constexpr std::size_t index_of() {
  static_assert(bcl::is_contained<Ty, Args...>::value,
    "Type is not contained in a list of arguments!");
  return detail::IndexOfImp<Ty, Args...>::index_of();
}

/// Returns number of types in the list of types Args.
template<class... Args> inline constexpr std::size_t size_of() {
  return detail::SizeOfImp<Args...>::size_of();
}

/// Returns number of elements in an array.
template <class T, std::size_t N>
constexpr inline size_t array_sizeof(T(&)[N]) {
  return N;
}

/// Applies a specified function to each value in the empty pack (this is stub).
template<class Function> inline void staticForeach(Function &&F) {}

/// Applies a specified function to each value in the pack.
template<class FirstTy, class... TailTy, class Function>
inline void staticForeach(Function &&F, FirstTy First, TailTy... Tail) {
  F(First);
  staticForeach(std::forward<Function>(F), Tail...);
}

/// Provides method to insert an element (Element) in a collection (Coll) which
/// support insert(Element &) method.
template<class Element, class Coll> struct SimpleInserter {
  inline static void insert(
    typename std::conditional<
      std::is_pointer<Element>::value, Element, Element &>::type E,
    Coll &C) { C.insert(E); }
};

/// Provides method to insert an element (Element) in a collection (Coll) which
/// supports push_back(Element &) method.
template<class Element, class Coll> struct PushBackInserter {
  inline static void insert(
    typename std::conditional<
      std::is_pointer<Element>::value, Element, Element &>::type E,
    Coll &C) { C.push_back(E); }
};

/// Provides method to count the number of inserted elements. Counter of type Ty
/// should support operator++() method.
template<class Element , class Ty> struct CountInserter {
  inline static void insert(
    typename std::conditional<
      std::is_pointer<Element>::value, Element, Element &>::type E,
    Ty &C) { ++C; }
};

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
static inline bool isWhitespace(unsigned char c) {
  switch (c) {
  case ' ': case '\t': case '\f': case '\v': case '\r': return true;
  default: return false;
  }
}

/// Add begining and ending quotes to a specified string
/// if quotes does not exist.
static inline std::string quote(const std::string &Str) {
  if (!(Str.size() > 1 && Str.front() == '"' && Str.back() == '"'))
    return '"' + Str + '"';
  return Str;
}

/// Add begining and ending quotes to a specified string
/// if quotes does not exist.
static inline std::string quote(std::string &&Str) {
  if (!(Str.size() > 1 && Str.front() == '"' && Str.back() == '"'))
    return '"' + Str + '"';
  return std::move(Str);
}

/// \brief Constructs an object of non-array type T and passes the given args
/// to the constructor of T.
///
/// This function is available since C++ 14, so this implementation is
/// convenient to use if only C++ 11 is available.
/// \return `unique_ptr<T>` which owns the object.
template <class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(Args &&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/// \brief Constructs an array of unknown bound T.
///
/// This function is available since C++ 14, so this implementation is
/// convenient to use if only C++ 11 is available.
/// \pre This function is overloaded for arrays of unknown bounds.
/// \param n size of the new array.
/// \return `unique_ptr<T>` which owns the object.
template <class T>
typename std::enable_if<
 std::is_array<T>::value && std::extent<T>::value == 0,
 std::unique_ptr<T>>::type
make_unique(size_t n) {
  return std::unique_ptr<T>(new typename std::remove_extent<T>::type[n]());
}

/// Construction of arrays of known bound is disallowed.
template <class T, class... Args>
typename std::enable_if<std::extent<T>::value != 0>::type
make_unique(Args &&...) = delete;

namespace detail {
template <typename T, typename... ArgT, std::size_t... Indexes>
std::unique_ptr<T> make_unique_piecewise(std::tuple<ArgT...> Args,
                                 std::index_sequence<Indexes...>) {
  return std::make_unique<T>(std::get<Indexes>(std::move(Args))...);
}
}

/// Construct an object of non-array type T and forwards elements of Args to the
/// constructor of T.
template <typename T, typename... ArgT>
std::unique_ptr<T> make_unique_piecewise(std::tuple<ArgT...> Args) {
  return detail::make_unique_piecewise<T>(std::move(Args),
                                          std::index_sequence_for<ArgT...>());
}

/// Exchanges contents of passed objects, this is useful if specified objects
/// haven't necessary operators available (e.g. private operator=, etc).
/// Note, this function does not swap external memory to which objects refer.
template<typename T> void swapMemory(T &LObj, T &RObj) {
  if (&LObj == &RObj)
    return;
  char* Tmp = nullptr;
  const int Size = sizeof(T) / sizeof(*Tmp);
  Tmp = new char[Size];
  std::memcpy(Tmp, &LObj, Size);
  std::memcpy(&LObj, &RObj, Size);
  std::memcpy(&RObj, Tmp, Size);
  delete[] Tmp;
}

/// Shrink a pair of values to a single value of a specified type if possible,
/// return true on success.
template<class FirstT, class SecondT, class T> bool shrinkPair(
    const FirstT &First, const SecondT &Second, T &Out) {
  char RawFirst[sizeof(FirstT)], RawSecond[sizeof(SecondT)];
  constexpr auto SizeOfT = sizeof(T);
  constexpr auto HalfSizeOfT = SizeOfT / 2;
  std::memset(RawFirst, 0, sizeof(FirstT));
  new(RawFirst) FirstT(First);
  for (std::size_t I = HalfSizeOfT; I < sizeof(FirstT) ; ++I)
    if (RawFirst[I] != 0)
      return false;
  std::memset(RawSecond, 0, sizeof(SecondT));
  new(RawSecond) SecondT(Second);
  for (std::size_t I = HalfSizeOfT; I < sizeof(SecondT); ++I)
    if (RawSecond[I] != 0)
      return false;
  std::memmove((char *)&Out, RawFirst, HalfSizeOfT);
  std::memmove((char *)&Out + HalfSizeOfT, RawSecond, HalfSizeOfT);
  return true;
}

/// Shrink a pair of values to a single value of a specified type if possible,
/// return true on success.
template<class FirstT, class SecondT, class T> bool shrinkPair(
    const std::pair<FirstT, SecondT> &Data, T &Out) {
  return shrinkPair(Data.first, Data.second, Out);
}

/// Restore a pair of values which have been previously shrinked to a specified
/// type `T`.
template<class FirstT, class SecondT, class T>
void restoreShrinkedPair(const T &Data, FirstT &First, SecondT &Second) {
  constexpr auto SizeOfT = sizeof(T);
  constexpr auto HalfSizeOfT = SizeOfT / 2;
  static_assert(sizeof(FirstT) >= HalfSizeOfT,
    "Too small target type of a first value!");
  static_assert(sizeof(SecondT) >= HalfSizeOfT ,
    "Too small target type of a second value!");
  char RawData[SizeOfT];
  new (RawData) T(Data);
  std::memset(&First, 0, sizeof(FirstT));
  std::memmove(&First, RawData, HalfSizeOfT);
  std::memset(&Second, 0, sizeof(SecondT));
  std::memmove(&Second, RawData + HalfSizeOfT, HalfSizeOfT);
}

}

#ifndef NULL
#ifdef __cplusplus
//! Нулевой указатель.
#define NULL 0
#else
//! Нулевой указатель.
#define NULL ( ( void *)0)
#endif
#endif

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

    //! Ннеопределенное значение.
    class Null
    {
    public:
        friend inline const Null & Undef( );

        //! Конструктор умолчания создает неопределенное значение.
        Null( ) { }
    };

    //! Возвращает неопределенное значение.
    inline const Null & Undef( ) { static const Null undef; return undef;}

//! Определяет неопределенное значение.
#define undef Utility::Undef( )

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

        //! Реализация проверки выполнимости орграничения.
        template< class Bool_ > struct ConstraintImp;

        //! Перегрузка для случая, когда ограничение выполнено.
        template< > struct ConstraintImp< Utility::True > { };

        //! Реализация оператора ветвления.
        /*! \tparam Condition_ Условие ветвления.
            \tparam TrueBranch_ Истинная ветвь ветвления.
            \tparam FalseBranch_ Ложная ветвь ветвления.
            \todo Добавить проверку того, что \c Condition_ - логический класс.
        */
        template< class Condition_, class TrueBranch_, class FalseBranch_ > struct IfImp;

        //! Реализация оператора ветвления для истинного условия ветвления.
        template< class TrueBranch_, class FalseBranch_ > struct IfImp< True, TrueBranch_, FalseBranch_ >
        {
            typedef TrueBranch_ Result; //!< Выбранная ветвь ветвления.
        };

        //! Реализация оператора ветвления для ложного условия ветвления.
        template< class TrueBranch_, class FalseBranch_ > struct IfImp< False, TrueBranch_, FalseBranch_ >
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

    //! Проверяет, что данное условие выполнено.
    /*! \tparam Condition_ Проверяемое условие.
    */
    template< class Condition_ > struct Constraint :
        public Detail::ConstraintImp< typename Condition_::Definition > { };

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

    //! Выполняет проверку того, что тип является скалярным.
    template< class What_ > struct IsScalar : public False { };

#define SCALAR_TYPE( type_) template< > struct IsScalar< type_ > : public True { };

    SCALAR_TYPE( short)
    SCALAR_TYPE( int)
    SCALAR_TYPE( long)
    SCALAR_TYPE( long long)
    SCALAR_TYPE( unsigned short)
    SCALAR_TYPE( unsigned int)
    SCALAR_TYPE( unsigned long)
    SCALAR_TYPE( unsigned long long)
    SCALAR_TYPE( float)
    SCALAR_TYPE( double)
    SCALAR_TYPE( long double)
    SCALAR_TYPE( char)
    SCALAR_TYPE( wchar_t)
    SCALAR_TYPE( Null)

    //! Выполняет проверку того, что тип является ссылкой.
    template< class What_ > struct IsReference : public False { };

    //! Специализация шаблонного класса, когда тип является ссылкой.
    template< class What_ > struct IsReference< What_ & > : public True{ };

    //! Выполняет проверку того, что тип является указателем.
    template< class What_ > struct IsPointer : public False { };

    //! Специализация шаблонного класса, когда тип является указателем.
    template< class What_ > struct IsPointer< What_ * > : public True{ };

    //! Определяет по типу указатель и ссылку.
    template< class What_ > struct TypeTraits
    {
        typedef What_ Type;
        typedef What_ & Reference;
        typedef const What_ & ReferenceC;
        typedef What_ * Pointer;
        typedef const What_ * PointerC;
    };

    //! Определяет по типу указатель и ссылку.
    template< class What_ > struct TypeTraits< What_ * >
    {
        typedef What_ * Type;
        typedef What_ & Reference;
        typedef const What_ & ReferenceC;
        typedef What_ * Pointer;
        typedef const What_ * PointerC;
    };

    //! Определяет указатель на заданный объект.
    template< class What_ > struct Pointer { typedef What_ * Declaration; };

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
    template< class Object_, template< class > class Classificator_, class Class_ > struct GroupTraits;

    //! Базовый класс для классов явно допускающих копирование.
    /*! \note Не нужно добавлять объявления конструкторов умолчания и операторов,
        так как они генерируются автоматически.
        Явное указание может повлиять на выполняемую компилятором оптимизацию,
        выполнние инлайн подстановки.
    */
    class Copyable { };

    /// \brief This prevents assignment of the derived classes.
    ///
    /// \attetion This is deprecated, use bcl::Unassignable instead.
    /// TODO (kaniandr@gmail.com) : Remove it when all dependent files will be
    /// rewritten.
    typedef bcl::Unassignable Unassignable;

    /// \brief This prevents copying of the derived classes.
    ///
    /// \attetion This is deprecated, use bcl::Unassignable instead.
    /// TODO (kaniandr@gmail.com) : Remove it when all dependent files will be
    /// rewritten.
    typedef bcl::Uncopyable Uncopyable;


    /*! \defgroup IObject Классы-обертки для объектов.
        \brief Классы-синонимы предоставляют единцый интерфейс для доступа к методам объектов.

        Не зависимо от того, расположен ли данный объект в динамической памяти или
        статической памяти, используются ли управляемый указатели, классы-обертки позволяют
        обращаться к методам объекта следующим образом: <tt> obj->Method(...) </tt>.
        Класс-обертки могут быть двух видов:
        - классы-синонимы (\c IObject) создают новый объект,
        способ создания объекта определяется параметром \c Object_.
        - классы-значения (\c IValue) позволяют контролировать наличие неопределенного значения для объекта

         \tparam Object_ Используемый объект.
         \attention В классе Object_ должен быть реализован конструктор копирования и оператор присваивания.
         Для классов-значения должен быть реализован констркутор умолчания.
        @{
    */
    namespace Detail
    {
        //! Реализация иинтерфейса доступа к методам объектов (см. \ref IObject).
        /*! Реализация для объектов, расположенных в статической памяти.
        */
        template< class Object_ > class IAccessorImp
        {
            typedef IAccessorImp< Object_ > This; //!< Текущий класс.

        public:
            typedef Object_ Declaration; //!< Описание объекта.
            typedef Object_ Object; //!< Исползуемый объект.
            typedef Object & Reference; //!< Ссылка на объект.
            typedef const Object & ReferenceC; //!< Ссылка на объект.

        public:
            //! Выполняет разыменование объекта.
            Reference operator* ( ) { return m_object; }

            //! Выполняет разыменование объекта.
            ReferenceC operator* ( ) const { return m_object; }

        protected:
             //! Создание нового объекта.
            static This New( ) { return This( ); }

            //! Удаление объекта.
            static void Delete( This & ) { }

            typedef Object_ Internal; //!< Внутреннее представление объекта.
            Internal m_object; //!< Используемый объект.
        };

        //! Реализация иинтерфейса доступа к методам объектов (см. \ref IObject).
        /*! Реализация для объектов, расположенных в динаической памяти
            и используемых с помощью средств ЯП С (* или ->).
        */
        template< class Object_ > class IAccessorImp< Object_ * >
        {
            typedef IAccessorImp< Object_ * > This; //!< Текущий класс.

        public:
            typedef Object_ * Declaration; //!< Описание объекта.
            typedef Object_ Object; //!< Исползуемый объект.
            typedef Object & Reference; //!< Ссылка на объект.
            typedef const Object & ReferenceC; //!< Ссылка на объект.

        public:
            //! Выполняет разыменование объекта.
            Reference operator*( ) { return *m_object;}

            //! Выполняет разыменование объекта.
          ReferenceC operator*( ) const { return *m_object;}

        protected:
            //! Конструктор умолчания.
            IAccessorImp( ) : m_object( NULL) { }

            //! Создание нового объекта.
            static This New( ) { return This( new Object); }

            //! Удаление объекта.
            static void Delete( This & object)
            {
                if ( !object.m_object)
                    return;
                delete object.m_object;
                object.m_object = NULL;
            }

            typedef Object * Internal; //!< Внутреннее представление объекта.
            Internal m_object; //!< Используемый объект.

        private:
            explicit IAccessorImp( Internal object) : m_object( object) { }
        };

        //! Интерфейс доступа к методам объектов (см. \ref IObject).
        template< class Object_ > class IAccessor :
            public IAccessorImp< Object_ >
        {
            typedef IAccessorImp< Object_ > Super; //!< Базовый класс.
            typedef IAccessor< Object_ > This; //!< Текущий класс.

        public:
            typedef typename Super::Object * Pointer; //!< Указатель на объект.
            typedef const typename Super::Object * PointerC; //!< Указатель на объект.

        public:
            //! Создание нового объекта.
            static This New( )
            {
                Super super = Super::New( );
                return static_cast< const This & >( super);
            }

            //! Удаление объекта.
            static void Delete( This & object)  { Super::Delete( object); }

        public:
            //! Выполняет разыменование объекта.
            Pointer operator->( ) { return &( **static_cast< Super *>( this)); }

            //! Выполняет разыменование объекта.
            PointerC operator->( ) const { return &( **static_cast< const Super * >( this)); }
        };
    }

    //! Класс-объект (см. \ref IObject).
    /*! \tparam Object_ Описание объекта.
        \tparam Nullable_ Признак допустимости (Utility::True или Utility::False) неопределнного значения.
    */
    template< class Object_, class Nullable_ > struct IObject;

    //! Специализация класса IObject для хранения величин, в случае недопустимости неопределенных значений.
    template< class Object_ > class IObject< Object_, Utility::False >
        : public Detail::IAccessor< Object_ >
    {
        typedef Detail::IAccessor< Object_ > Super; //!< Интерфейс доступа.

    public:
        typedef typename Super::Declaration Declaration; //!< Описание объекта.
        typedef Utility::False Nullable; //!< Признак допустимости неопределнного значения.
        typedef IObject< Declaration, Nullable > This; //!< Текушщий класс.
        typedef IObject< Declaration, Utility::True > Opposite; //!< Аналогичный объект допускающий неопределенное значение.

    public:
        //! Создание нового объекта.
        static This New( ) { return Super::New( ); }

        //! Удаление объекта.
        static void Delete( This & object)  { Super::Delete( object); }

    public:
        //! Конструктор копирования.
        IObject( const This &object) : Super( object) { }

        //! Конструктор копирования.
        IObject( const Opposite &object) : Super( object) { *object; }

        //! Присваивает новое значение.
        This & operator= ( const Opposite &object)
        {
            Super::m_object = object;
            *object;

            return this;
        }

        //! Признак определенного значения объекта.
        operator bool ( ) const { return true; }

    protected:
        //! Конструктор умолчания.
        IObject( ) { }

    private:
        //! Конструктор копирования.
        IObject( const Super &object) : Super( object) { }
    };

    //! Специализация класса IObject для хранения величин, в случае допустимости неопределенных значений.
    template< class Object_ > class IObject< Object_, Utility::True >
        : public Detail::IAccessor< Object_ >
    {
        typedef Detail::IAccessor< Object_ > Super; //!< Интерфейс доступа.

    public:
        typedef typename Super::Declaration Declaration; //!< Описание объекта.
        typedef typename Super::Reference Reference; //!< Ссылка на объект.
        typedef typename Super::ReferenceC ReferenceC; //!< Ссылка на объект.
        typedef typename Super::Pointer Pointer; //!< Указатель на объект.
        typedef typename Super::PointerC PointerC; //!< Указатель на объект.
        typedef Utility::True Nullable; //!< Признак допустимости неопределнного значения.
        typedef IObject< Declaration, Nullable > This; //!< Текушщий класс.
        typedef IObject< Declaration, Utility::False > Opposite; //!< Аналогичный объект недопускающий неопределенное значение.

    public:
        //! Создание нового объекта.
        static This New( ) { return This( Super::New( )); }

        //! Удаление объекта.
        static void Delete( This & object) { Super::Delete( object); object.Reset( ); }

    public:
        //! Конструктор умолчания.
        IObject( ) : m_ptr( NULL) { }

        //! Конструктор копирования.
        IObject( const This &object) :
            This( static_cast< Super & >( object)) { }

        //! Конструктор копирования.
        IObject( const Opposite &object) :
            This( static_cast< Super & >( object)) { }

        //! Выполняет разыменование объекта.
        Reference operator*( ) { return **m_ptr;}

        //! Выполняет разыменование объекта.
        ReferenceC operator*( ) const { return *m_ptr;}

        //! Выполняет разыменование объекта.
        Pointer operator->( ) { return &*this; }

        //! Выполняет разыменование объекта.
        PointerC operator->( ) const { return &*this; }

        //! Присваивает новое значение.
        This & operator= ( const This &object)
        {
            if ( !object)
            {
                Reset( );
                return this;
            }

            Super::m_object = object;
            m_ptr = this;

            return this;
        }

        //! Присваивает новое значение.
        This & operator= ( const Opposite &object)
        {
            Super::m_object = object;
            m_ptr = this;

            return this;
        }

        //! Признак определенного значения объекта.
        operator bool ( ) const { return m_ptr != NULL; }

        //! Устанавливает неопределенное значение.
        inline void Reset( ) {  m_ptr = NULL; }

    private:
        //! Конструктор копирования.
        IObject( const Super &object) : Super( object), m_ptr( this) { }

    private:
        Super *m_ptr; //!< Указатель на объект.
    };

    //! Специализация класса IObject для хранения указателей, в случае допустимости неопределенных значений.
    template< class Object_ > class IObject< Object_ *, Utility::True >
        : public Detail::IAccessor< Object_ * >
    {
        typedef Detail::IAccessor< Object_ * > Super; //!< Базовый класс.

    public:
        typedef typename Super::Declaration Declaration; //!< Описание объекта.
        typedef Utility::True Nullable; //!< Признак допустимости неопределнного значения.
        typedef IObject< Declaration, Nullable > This; //!< Текущий класс.
        typedef IObject< Declaration, Utility::False > Opposite; //!< Аналогичный объект, недоускающий неопределенное значение.

    public:
        //! Создание нового объекта.
        static This New( ) { return Super::New( ); }

        //! Удаление объекта.
        static void Delete( This & object) { Super::Delete( object); object.Reset( ); }

    public:
        //! Конструктор умолчания.
        IObject( ) { }

        //! Конструктор копирования.
        IObject( const This &object) : Super( object) { }

        //! Конструктор копирования.
        IObject( const Opposite &object) : Super( object) { }

        //! Присваивает новое значение.
        This & operator= ( const Opposite &object)
        {
            Super::m_object = object;

            return this;
        }

        //! Признак определенного значения объекта.
        operator bool ( ) const { return Super::m_object != NULL; }

        //! Устанавливает неопределенное значение.
        void Reset( ) { Super::m_object = NULL; }

    private:
        //! Конструктор копирования.
        IObject( const Super &object) : Super( object) { }
    };
    //@}
}

#endif//BCL_UTILITY_H

//===--- Json.h ------------- JSON String Serializer ------------*- C++ -*-===//
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
// This file implements functionality to parse string contains data in JSON
// format and convert this string to an appropriate high-level representation.
// Let us call this string JSON string and appropriate high-level
// representation JSON object. Backward conversion is also supported. That
// means that JSON object can be unparsed to a JSON string.
//
// The main functionality is available from json::Parser class. Different types
// are supported, for example, scalar types (int, long, float, etc.),
// character strings (char *, std::string), arrays (Ty *), some classes
// (bcl::Diagnostic) and other.
//
// Some special supported type is bcl::StaticMap. To convert some JSON string
// to this type it is necessary to declare appropriate type previously.
//
// If JSON objects have special static method name which returns an identifier
// of each object in a string format and if this identifier is used in
// JSON string, the json::Parser<...>::parse() method can determine an
// appropriate JSON object in objects specified as a parameters for Parser
// template. This method returns json::Object. Note that to make available this
// search JSON objects should inherit json::Object.
//
// To implement conversions for other types or separate cells in bcl::StaticMap
// it is possible to specialize json::Traits and json::CellTraits
// templates. If there is not built-in static method name() in a cell in some
// bcl::StaticMap it is possible to implement it in json::CellTraits
// specialization.
//
// If some errors occurred conversion methods should implement garbage
// collection.
//
// To build JSON string for a JSON object json::Parser<...>::unparse() method
// can be used. Note that parse() treats pointer as array or scalar depending
// on the to context but unparse() treats pointer always as an array, except
// pointers to char (char *). This is due to there is no any way to determine
// size of array.
//
//===----------------------------------------------------------------------===//
//
// Now let us see some usage example.
//
// 1. The simplest example is:
// \code
//   json::Parser<> P(R"j({"0":1, "1":2})j");
//   int * Array;
//   P.parse(Array);
//   for (int I = 0; I < 2; ++I)
//     std::cout << Array[I] << ' ';
// \endcode
// An output is: 1 2.
//
// 2. The following code is parsed JSON string "{\"Name\": "Jon", \"Age\": 1}"
// and prints it to STDOUT. Use Human Obj; Obj[Human::Name] to access a name.
// \code
//   JSON_OBJECT_BEGIN(Human)
//   JSON_OBJECT_PAIR_2(Human, Name, std::string, Age, unsigned)
//   JSON_OBJECT_END(Human)
//   JSON_DEFAULT_TRAITS(::, Human)
//
//   json::Parser<> P(R"j({"Name": "Jon", "Age": 1})j");
//   Human O;
//   if (P.parse<Human>(O)) {
//     std::cout << O[Human::Name] << ":" << O[Human::Age] << std::endl;
//
// 3. If JSON string contains identifier a parse can determine how to parse
// a specified string. Identifier is a pair of "name" and a value,
// for example "Human" in a JSON string.
// \code
//   JSON_OBJECT_BEGIN(Human)
//   JSON_OBJECT_ROOT_PAIR_2(Human, Name, std::string, Age, unsigned)
//     Human() : JSON_INIT_ROOT {}
//   JSON_OBJECT_END(Human)
//   JSON_DEFAULT_TRAITS(::, Human)
//
//   JSON_OBJECT_BEGIN(Dog)
//   JSON_OBJECT_ROOT_PAIR(Dog, Name, std::string)
//   JSON_OBJECT_END(Dog)
//   JSON_DEFAULT_TRAITS(::, Dog)
//
//   json::Parser<Humna, Dog> P(R"j({"name": "Human", "Name": "Jon", "Age": 1})j");
//   auto O = P.parse();
//   if (O && O.is<Human>()) {
//     Human &OH = O->as<Humna>();
//     std::cout << OH[Human::Name] << OH[Human::Age] << std::endl;
//   }
// \endcode
// Note, that JSON_OBJECT_ROOT_PAIR should be used instead of JSON_OBJECT_PAIR.
// It is also necessary to add at least default constructor and use
// JSON_INIT_ROOT in constructor definition.
//
// 4. It is also possible to add other constructors and typedefs for value types
// and define an object in a namespace:
// \code
// namespace People {
//   JSON_OBJECT_BEGIN(Human)
//   JSON_OBJECT_ROOT_PAIR_2(Human, Name, Age)
//     Human(const std::string &N, unsigned A) :
//       JSON_INIT(Human, N, A), JSON_INIT_ROOT {}
//     using NameTy = JSON_VALUE_TYPE(Human, Name);
//     using AgeTy = JSON_VALUE_TYPE(Human, Age);
//   JSON_OBJECT_END(::, Human)
// }
// JSON_DEFAULT_TRAITS(People::, Human)
// \endcode
//
// 5. If there were errors while a string has been parsed it is possible to
// investigate these errors.
// \code
//   if (!P.parse<Human>(O)) {
//     for (auto &Err : P.errors())
//       std::cerr << Err << "\n";
//     if (P.errors().internal_size() > 0)
//       std::cerr << "json error: " << P.errors().internal_size() <<
//       " internal errors\n";
//   }
// 6. To unparse JSON object O use the following code:
// \code
//   // Do not add identifier ("name": "Human") to the result.
//   // This works in case of sub-object (JSON_OBJECT_PAIR was used instead of
//   // JSON_OBJECT_ROOT_PAIR) and in case of top-level objects.
//   Human O;
//   json::Parser<Human>::unparse(O);
//
//   // Add identifier to the result. This works only in case of top-level
//   // objects.
//   Human O;
//   json::Parser<Human>::unparseAsObject(O);
//
//   // Add identifier to the result. This works only in case of top-level
//   // objects.
//   Human OH;
//   Object &O = OH;
//   Parser<Human>::unparse(O);
// \endcode
//
// 7. If you want to have more opportunities to customize JSON-object definition
// it is possible to define new object in the following way:
// \code
//   // Let us begin description of a JSON-object 'Human' in a global namespace.
//   JSON_OBJECT_BEGIN(Human)
//     // The object contains two name-value pairs which represents human name
//     // and its age. Here we specify type of a value in each pair.
//     JSON_VALUE(Name, std::string)
//     JSON_VALUE(Age, unsigned)
//
//     // Now, we specify list of all name-value pairs which should be stored
//     // in JSON. We use JSON_OBJECT_ROOT to indicate that the current object
//     // is top-level object. If we want to use this object as a sub-object
//     // only we can use JSON_OBJECT instead.
//     JSON_OBJECT_ROOT(Human, Name, Age)
//
//     // Now, we specify name-value pairs which can be accessed in C++ code in
//     // the following way Obj[Human::Name] where 'Obj' has type 'Human'.
//     JSON_ACCESS(Human, Name)
//     JSON_ACCESS(Human, Age)
//
//     // Arbitrary C++ code can be written here. It becomes a part of
//     // the Human class. For example, it is possible to add constructor.
//     // Attention, JSON_INIT_ROOT must be always used in case of
//     // top-level objects! It is necessary to use it in any constructors
//     // of a top-level objects (including default).
//     // So, top-level object can not have an implicit default constructor.
//     // Hence, a simplest way to define constructor of a top-level object is
//     // 'Human() : JSON_INIT_ROOT {}'.
//     // Attention, do not use JSON_INIT_ROOT in case of sub-objects!
//     Human(const std::string &N, unsigned A) :
//       JSON_INIT(N, A), JSON_INIT_ROOT {}
//
//     // It is also possible to use type of a value in a name-value pair.
//     using NameTy = JSON_VALUE_TYPE(Human, Name);
//     using AgeTy = JSON_VALUE_TYPE(Human, Age);
//
//   // Here, we finalize description of objects.
//   JSON_OBJECT_END(Human)
//   // An finally, it is necessary to describe how the object should be
//   // parsed/unparsed. Here we use default capabilities, however, it could be
//   // also customized manually.
//   JSON_DEFAULT_TRAITS(::, Human)
// \endcode
//
// 8. It is also possible to define JSON-object manually.
// \code
//   namespace detail {
//   struct ExampleObject {
//     struct Text {
//       static inline const std::string & name() {
//         static const std::string N("Text");
//         return N;
//       }
//       using ValueType = const char *;
//     };
//   };
//   }
//   struct ExampleObject : public bcl::StaticMap<detail::ExampleObject::Text> {
//     static constexpr detail::ExampleObject::Text Text =
//       detail::ExampleObject::Text();
//   };
//   template<> struct json::Traits<ExampleObject> :
//     public json::Traits<bcl::StaticMap<::detail::ExampleObject::Text>> {};
// \endcode
// The previous code defines some JSON object ExmpaleObject with a single field
// Text.
//===----------------------------------------------------------------------===//

#ifndef BCL_JSON_H
#define BCL_JSON_H

#include "cell.h"
#include "Diagnostic.h"
#include "utility.h"
#include <cctype>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stack>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#define JSON_ERROR_1 "unexpected end of string"
#define JSON_ERROR_2 "unexpected character '%c' expected '%c'"
#define JSON_ERROR_3 "unknown json string, identifier '%s' is not found"
#define JSON_ERROR_4 "unexpected character '%c' identifier expected"
#define JSON_ERROR_5 "unexpected character '%c' identifier, number or keyword expected"
#define JSON_ERROR_6 "value conversion error"
#define JSON_ERROR_7 "uninitialized elements in array"
#define JSON_ERROR_8 "target object type does not support duplicate of keys"
#define JSON_ERROR_9 "illegal value"
#define JSON_ERROR(C) C, JSON_ERROR_##C

#define JSON_OBJECT_BEGIN(Object_) \
namespace json_ { struct Object_##Impl {

/// Provides static name() method which.
#define JSON_NAME(Name) \
static inline const std::string & \
name() { static const std::string N(#Name); return N;}

///\brief Specifies a way to access a value of a JSON name-value pari.
///
/// Usage example:
///   Object_ Val;
///   Val[Object_::Name__] = ...
#define JSON_ACCESS(Object_, Name_) \
static constexpr json_::Object_##Impl::Name_ Name_ \
= json_::Object_##Impl::Name_();

/// Specifies a JSON name-value pair. Instead of a concrete value its type should
/// be specified.
#define JSON_VALUE(Name_, Type_) \
struct Name_ { JSON_NAME(Name_) using ValueType = Type_; };

/// Defines structure of a sub-object.
#define JSON_OBJECT(Object_, ...) \
  using Base = bcl::StaticMap<__VA_ARGS__>; }; } \
struct Object_ : public json_::Object_##Impl::Base {

/// Defines structure of a top-level object.
#define JSON_OBJECT_ROOT(Object_, ...) \
  using Base = bcl::StaticMap<__VA_ARGS__>; }; } \
struct Object_ : \
  public json_::Object_##Impl::Base, public ::json::Object { \
  JSON_NAME(Object_)

/// Initializes object with a list of values.
#define JSON_INIT(Object_, ...) json_::Object_##Impl::Base(__VA_ARGS__)

/// Initializes top-level object.
#define JSON_INIT_ROOT ::json::Object(name())

/// Type of a value with name Name_ in a name-value pair inside object Object_.
#define JSON_VALUE_TYPE(Object_, Name_) \
json_::Object_##Impl::Name_::ValueType

/// Ends description of JSON-object.
#define JSON_OBJECT_END(Object_) };

/// Specifies that a JSON-object should be parsed/unparsed in a default way.
#define JSON_DEFAULT_TRAITS(namespace_, Object_) \
namespace json { \
template<> struct Traits<namespace_ Object_> : \
  public Traits<namespace_ json_::Object_##Impl::Base> {}; \
}

namespace json {
/// This is a base class for all JSON objects which can be obtained when
/// a string represented JSON is parsed.
///
/// \attention All JSON objects must inherit this class and implement a static
/// method `static ObjectName name()`.
class Object {
public:
  /// Identifier of an object.
  typedef std::string ObjectName;

  /// Default constructor.
  explicit Object(const ObjectName &Name) : mName(Name) {}

  /// Virtual destructor.
  virtual ~Object() {}

  Object(const Object &) = default;
  Object & operator=(const Object &) = default;
  Object(Object &&) = default;
  Object & operator=(Object &&) = default;

  /// Returns identifier of an object.
  const ObjectName & getName() const noexcept { return mName; }

  /// Returns true if this object has a specified type Ty.
  template<class Ty> bool is() const { return Ty::name() == mName; }

  /// \brief Casts object to a specified type.
  ///
  /// \pre This object must have a specified type, otherwise behavior
  /// is undefined.
  template<class Ty> Ty & as() { return static_cast<Ty &>(*this); }

  /// \brief Casts object to a specified type.
  ///
  /// \pre This object must have a specified type, otherwise behavior
  /// is undefined.
  template<class Ty> const Ty & as() const {
    return static_cast<const Ty &>(*this);
  }
private:
  ObjectName mName;
};

/// Type of tokens which may occur in a JSON string.
enum class Token : char {
  LEFT_BRACKET = '[',
  RIGHT_BRACKET = ']',
  LEFT_BRACE = '{',
  RIGHT_BRACE = '}',
  COMMA = ',',
  COLON = ':',
  QUOTE = '"',
  ESCAPE = '/',
  DOT = '.',
  PLUS = '+',
  MINUS = '-',
  NUMBER = 'n',
  IDENTIFIER = 'i',
  KEYWORD = 'k',
  INVALID = '\0',
};

/// List of keywords which may occur in a JSON string.
enum class Keyword : uint8_t {
  TRUE = 0,
  FALSE,
  NO_VALUE,
};

/// List of keywords which may occur in a JSON string.
const char *const KeywordTable[] = {"true", "false", "null"};

inline std::string_view toString(Keyword K) noexcept {
  assert(0 <= static_cast<std::underlying_type_t<Keyword>>(K) &&
         static_cast<std::underlying_type_t<Keyword>>(K) <
             sizeof KeywordTable / sizeof KeywordTable[0] &&
         "Unknown keyword!");
  return KeywordTable[static_cast<std::underlying_type_t<Keyword>>(K)];
}

/// Representation of a JSON string.
typedef std::string String;

/// Position in a JSON string.
typedef std::string::size_type Position;

/// This is a lexer for a JSON string.
class Lexer: private bcl::Uncopyable {
  /// Checks whether a specified character Ch is a quote.
  static inline bool isQuote(char Ch) noexcept {
    return Ch == static_cast<char>(Token::QUOTE);
  }

  /// Checks whether a specified character Ch is escape ('\').
  static inline bool isEscape(char Ch) noexcept {
    return Ch == static_cast<char>(Token::ESCAPE);
  }

  /// Checks whether a specified character Ch is dot.
  static inline bool isDot(char Ch) noexcept {
    return Ch == static_cast<char>(Token::DOT);
  }

  /// Checks whether a specified character Ch identifies a sign of a number.
  static inline bool isSign(char Ch) noexcept {
    return Ch == static_cast<char>(Token::MINUS) ||
      Ch == static_cast<char>(Token::PLUS);
  }

  /// Checks whether a specified character Ch is a left brace.
  static inline bool isLeftBrace(char Ch) noexcept {
    return Ch == static_cast<char>(Token::LEFT_BRACE);
  }

  /// Checks whether a specified character Ch is a right brace.
  static inline bool isRightBrace(char Ch) noexcept {
    return Ch == static_cast<char>(Token::RIGHT_BRACE);
  }

  /// Checks whether a specified character Ch is a left bracket.
  static inline bool isLeftBracket(char Ch) noexcept {
    return Ch == static_cast<char>(Token::LEFT_BRACKET);
  }

  /// Checks whether a specified character Ch is a right bracket.
  static inline bool isRightBracket(char Ch) noexcept {
    return Ch == static_cast<char>(Token::RIGHT_BRACKET);
  }

  /// State of a lexer.
  struct State {
    State(Position S, Position E, Position N, Token T, bool IsInt) :
      mStart(S), mEnd(E), mNext(N), mToken(T), mIsIntegral(IsInt) {}

    Position mStart = 0;
    Position mEnd = 0;
    Position mNext = 0;
    Token mToken;
    bool mIsIntegral = false;
  };

public:
  /// Constructs a lexer for a specified JSON string.
  explicit Lexer(const String &JSON) : mJSON(JSON), mErrors("json error") {}

  /// Goes to a next token in a JSON string.
  ///
  /// The token is represented by characters in a range [start(), end()].
  /// To access a character following the token use next().
  /// \return `true` if JSON string has been successfully traversed. If errors
  /// have been occurred they will be stored in errors() container and this
  /// returns `false`.
  bool goToNext() {
    for (; mNext < mJSON.size() && std::isspace(mJSON[mNext]); ++mNext);
    mToken = Token::INVALID;
    if (mNext >= mJSON.size()) {
      mErrors.insert(JSON_ERROR(1), mNext);
      mStart = mEnd = mNext = mJSON.size();
      return false;
    }
    mStart = mEnd = mNext;
    if (isQuote(mJSON[mNext])) {
      for (++mNext; mNext < mJSON.size(); ++mNext) {
        if (isQuote(mJSON[mNext]) && !isEscape(mJSON[mNext - 1])) {
          mEnd = mNext++;
          mToken = Token::IDENTIFIER;
          return true;
        }
      }
      mErrors.insert(JSON_ERROR(1), mStart);
      mStart = mEnd = mNext = mJSON.size();
      return false;
    } else if (std::isdigit(mJSON[mNext]) || isSign(mJSON[mNext])) {
      mToken = Token::NUMBER;
      mIsIntegral = true;
      for (++mNext; mNext < mJSON.size(); ++mNext) {
        if (std::isdigit(mJSON[mNext]))
          continue;
        if (isDot(mJSON[mNext])) {
          if (!mIsIntegral)
            break;
          mIsIntegral = false;
        } else {
          break;
        }
      }
      mNext = mNext < mJSON.size() ? mNext : mJSON.size();
      mEnd = mNext - 1;
      return true;
    } else if (std::isalpha(mJSON[mNext])) {
      if (auto K{[this](std::string_view S) {
            for (std::size_t I = 0,
                             EI = sizeof KeywordTable / sizeof KeywordTable[0];
                 I < EI; ++I) {
              std::string_view Current{KeywordTable[I]};
              if (S.substr(0, Current.size()) == Current) {
                mNext = mStart + Current.size();
                mEnd = mNext - 1;
                mToken = Token::KEYWORD;
                mKeyword = static_cast<Keyword>(I);
                return true;
              }
            }
            return false;
          }(mJSON.data() + mNext)})
        return true;
    }
    mEnd = mNext;
    mToken = static_cast<Token>(mJSON[mStart]);
    ++mNext;
    return true;
  }

  /// \brief Checks that current token is a special character Ch.
  ///
  /// \return If a current token is not a special character Ch this method
  /// returns false and add appropriate error in the errors collection.
  bool checkSpecial(Token Ch) {
    if (is(Ch))
      return true;
    mErrors.insert(JSON_ERROR(2), mStart, mJSON[mStart], Ch);
    return false;
  }

  /// \brief Checks that current token is an identifier.
  ///
  /// \return If a current token is not an identifier this method
  /// returns false and add appropriate error in the errors collection.
  bool checkIdentifier() {
    if (is(Token::IDENTIFIER))
      return true;
    mErrors.insert(JSON_ERROR(4), mStart, mJSON[mStart]);
    return false;
  }

  /// \brief Checks that current token is an identifier, a number or a keywrod
  /// ('true', 'false', 'null').
  ///
  ///
  /// \return If a current token is not an identifier and a number this method
  /// returns false and add appropriate error in the errors collection.
  bool checkValue() {
    if (is(Token::IDENTIFIER) || is(Token::NUMBER) ||
        isKeyword(Keyword::TRUE) || isKeyword(Keyword::FALSE) ||
        isKeyword(Keyword::NO_VALUE))
      return true;
    mErrors.insert(JSON_ERROR(5), mStart, mJSON[mStart]);
    return false;
  }

  /// Skips all characters in a JSON string between braces or brackets,
  /// return false if some errors have been occurred.
  bool skipInternal() {
    Token Last;
    if (is(Token::LEFT_BRACKET))
      Last = Token::RIGHT_BRACKET;
    else if (checkSpecial(Token::LEFT_BRACE))
      Last = Token::RIGHT_BRACE;
    else
      return false;
    int Level = 0;
    while (goToNext()) {
      if ((isRightBrace(mJSON[mStart]) || isRightBracket(mJSON[mStart])) &&
          Level-- == 0)
        return checkSpecial(Last);
      if (isLeftBrace(mJSON[mStart]) || isLeftBracket(mJSON[mStart]))
        ++Level;
    }
    return false;
  }

  /// Resets current lexer position.
  void resetPosition() noexcept {
    mStart = mEnd = mNext = 0;
    mToken = Token::INVALID;
  }

  /// Sets lexer position and parses the first token at the new position.
  void setPosition(Position Start) {
    mStart = mEnd = mNext = Start;
    mToken = Token::INVALID;
    goToNext();
  }

  /// Saves the current position in an internal stack, it can be restored with
  /// restorePosition() method.
  void storePosition() {
    mStates.emplace(State(mStart, mEnd, mNext, mToken, mIsIntegral));
  }

  /// Restores the last stored position. If there is no stored positions do
  /// nothing.
  void restorePosition() {
    if (mStates.empty())
      return;
    mStart = mStates.top().mStart;
    mEnd = mStates.top().mEnd;
    mNext = mStates.top().mNext;
    mToken = mStates.top().mToken;
    mIsIntegral = mStates.top().mIsIntegral;
  }

  /// \brief Returns start position of a current token in the JSON string.
  Position start() const noexcept { return mStart; }

  /// \brief Returns end position of a current token in the JSON string.
  Position end() const noexcept { return mEnd; }

  /// Returns position of a character following a current token in the
  // JSON string.
  Position next() const noexcept { return mNext; }

  /// Returns true if a current token is equal to a specified one.
  bool is(Token Token) const noexcept { return mToken == Token; }

  /// Returns true if a current token is an integral number.
  bool isIntegral() const noexcept {
    return mToken == Token::NUMBER && mIsIntegral;
  }

  /// Returns true if a current token is an floating point number.
  bool isFloatingPoint() const noexcept {
    return mToken == Token::NUMBER && !mIsIntegral;
  }

  /// Return true if a current token is a specified keyword.
  bool isKeyword(Keyword K) const noexcept {
    return is(Token::KEYWORD) && mKeyword == K;
  }

  /// Returns container of errors.
  bcl::Diagnostic & errors() noexcept { return mErrors; }

  /// Returns container of errors.
  const bcl::Diagnostic & errors() const noexcept { return mErrors; }

  /// Returns true if errors have been occurred, internal errors are
  /// also considered.
  bool hasErrors() const {
    return !errors().empty() || errors().internal_size() > 0;
  }

  /// Discards limiting quotes of the current token, if it is an identifier, or
  /// does nothing.
  std::pair<Position, Position> discardQuote() const noexcept {
    return is(Token::IDENTIFIER) ?
      std::make_pair(start() + 1, end() - 1) :
      std::make_pair(start(), end());
  }

  /// Returns a JSON string.
  const String & json() const noexcept { return mJSON; }

private:
  String mJSON;
  bcl::Diagnostic mErrors;
  Position mStart = 0;
  Position mEnd = 0;
  Position mNext = 0;
  Token mToken;
  bool mIsIntegral = false;
  Keyword mKeyword = Keyword::NO_VALUE;
  std::stack<State> mStates;
};

/// \brief This implements methods to convert value in a JSON string to
/// specified type and to assign it to a specified destination.
///
/// This class should be specialized by different types. There are some
/// default specialization for simple types for other types conversion
/// must be specialized manually.
///
///
/// The following static methods should be implemented:
/// - static bool parse(Ty &, Lexer &) -
///     Converts value from a JSON string to a type Ty and stores it
///     in to a specified destination.
///     Lexer parameter points to the first value token.
///     This returns true on success.
/// - static bool parse(Ty &, Lexer &, std::pair<Position, Position>) -
///     Converts value from a JSON string to a type Ty and stores it
///     in to a specified destination.
///     Lexer parameter points to the first value token.
///     If value is represented as a map {...} then the last parameter is
///     a position of a value key in a JSON string
///     (first and end character is quotes). In case of arrays [] the last
///     parameter is a pair of (0, number), where number is a number of
///     currently evaluated element of the array.
///     This returns true on success.
/// - static void unparse(String &JSON, const Ty &) -
///     Converts value to string and stores it ina JSON string.
///
/// This last parse method is necessary to evaluate compound types, i.e. types
/// with values represented as {"K1":V1, ..., "KN":VN} or [V1, ..., VN].
/// The following implementation is recommended. The parse(Ty &, Lexer &) method
/// will be invoked to convert compound value to Ty and store it in to the
/// destination specified by the first parameter. In case of compound type this
/// method invokes Parser<>::traverse() method to traverses all pair "KI":VI
/// (or only VI in case of arrays) and invokes for each pair the method
/// parse(Ty *, Lexer &, std::pair<Position, Position>).
/// The last parameter will be a position of "KI" or a number of currently
/// evaluated element of array.
template<class Ty> struct Traits {
  typedef typename Ty::UnknownTraitsError ValueType;
};

/// \brief This implements methods to convert value in a JSON string to
/// specified cell in a static map.
///
/// This class can be specialized by different keys in a static map.
/// By default this uses implementation of Traits for a type of
/// data stored in a cell. Note that some specialization can traverse
/// tokens in a JSON string. For example, see specialization for pointers or
/// bcl::StaticMap.
template<class CellKey> struct CellTraits {
  typedef typename CellKey::ValueType ValueType;
  inline static bool parse(ValueType &Dest, Lexer &Lex)
      noexcept(
        noexcept(Traits<ValueType>::parse(Dest, Lex))) {
    return Traits<ValueType>::parse(Dest, Lex);
  }
  inline static void unparse(String &JSON, const ValueType &Obj)
      noexcept(
        noexcept(Traits<ValueType>::unparse(JSON, Obj))) {
    Traits<ValueType>::unparse(JSON, Obj);
  }
  inline static typename std::result_of<
    decltype(&CellKey::name)()>::type name()
      noexcept(noexcept(CellKey::name())) {
    return CellKey::name();
  }
};

/// \brief This class parses a string which represents JSON and converts it to
/// an appropriate representation.
///
/// \tparam Objects List of JSON objects supported by parser. Each object
/// should inherited bcl::StaticMap and json::Object.
template<class... Objects> class Parser {
  /// List of supported JSON objects.
  typedef bcl::TypeList<Objects...> ObjectTypeList;

  /// \brief This parses a specified JSON string and converts it to
  /// a specified type.
  ///
  /// There are two ways to use this functor. If a target type is known it is
  /// possible to use static method parse(), otherwise this functor can be
  /// passed to a bcl::TypeList::for_each() method. In the last case
  /// bcl::TypeList should comprise different target types and appropriate type
  /// will be determined by name. Note that in this case all target type must
  /// propose a static `Object::ObjectName name()` method.
  class ParseFunctor {
  public:
    /// Converts JSON string to a specified Ty.
    template<class Ty> static bool parse(Ty &Obj, Lexer &Lex) {
      Lex.resetPosition();
      Lex.goToNext();
      if (!Traits<Ty>::parse(Obj, Lex)) {
        Lex.errors().insert(JSON_ERROR(6), Lex.start());
        return false;
      }
      if (Lex.next() < Lex.json().size()) {
        Lex.goToNext();
        Lex.checkSpecial(Token::COMMA);
        return false;
      }
      return true;
    }

    /// Creates functor to convert JSON string to a type with a specified name.
    ParseFunctor(Object::ObjectName Name, Lexer &Lex) :
      mName(Name), mLex(Lex) {}

    /// Converts JSON string to a specified Ty if it has an appropriate name.
    template<class Ty> void operator()() {
      if (Ty::name() != mName)
        return;
      auto Obj = std::unique_ptr<Ty>(new Ty);
      if (!parse(*Obj, mLex))
        return;
      mObject = std::move(Obj);
    }

    /// Returns unique pointer to created object or nullptr.
    std::unique_ptr<Object> stealObject() noexcept {
      return std::move(mObject);
    }

  private:
    Object::ObjectName mName;
    Lexer &mLex;
    std::unique_ptr<Object> mObject;
  };

  /// \brief This unparses JSON object to a JSON string.
  ///
  /// There are two ways to use this functor. If type of an object is known it
  /// is possible to use static method unparse(), otherwise this functor can be
  /// passed to a bcl::TypeList::for_each() method. In the last case
  /// bcl::TypeList should comprise different target types and appropriate type
  /// will be determined by name. Note that in this case all target type must
  /// propose a static `Object::ObjectName name()` method.
  class UnparseFunctor {
  public:
    /// Unparses JSON object of a specified type.
    template<class Ty> static String unparse(const Ty &Obj) {
      String JSON;
      Traits<Ty>::unparse(JSON, Obj);
      return JSON;
    }

    /// Creates functor to unparse a specified JSON object.
    explicit UnparseFunctor(const Object &Obj, const char *NameKey)
      : mObj(Obj), mNameKey(NameKey) {
      assert(NameKey && "Identifier of a JSON object must not be null!");
    }

    /// Unparses JSON object if it has a specified type Ty.
    template<class Ty> void operator()() {
      if (!mObj.is<Ty>())
        return;
      mString = unparse(mObj.as<Ty>());
      assert(mString.front() == '{' &&
        "The first character in a JSON string must be '{'");
      assert(mString.back() == '}' &&
        "The last character in a JSON string must be '}'");
      std::string Name("\"");
      Name += mNameKey;
      Name += "\":\"";
      Name += mObj.getName() + "\"";
      if (mString.find(':') != String::npos)
        Name += ',';
      mString.insert(1, Name);
    }

    /// Returns unparsed object.
    const String & getString() const noexcept { return mString; }
  private:
    String mString;
    const Object &mObj;
    const char *mNameKey;
  };

public:
  /// \brief Traverses all pairs of keys and value in a string bounded with
  /// left and right braces.
  ///
  /// This is a utility method which is used to parse JSON string. It should be
  /// used to add support for additional value conversions.
  /// \tparam CT This is specialization of CellTraits or Traits
  /// templates. For each pair of keys and values the method CT::parse() will be
  /// called.
  /// \tparam Ty This is a type of JSON object where the converted values
  /// must be stored.
  /// \param [in, out] Dest JSON object which is used to store converted value.
  /// \param [in, out] Lex Lexer which should point to a Token::LEFT_BRACE or
  /// Token::LEFT_BRACKET tokens. The method will traverse tokens to
  /// the Token::RIGHT_BRACE or Token::RIGHT_BRACKET.
  /// \return True on success or false if some errors have been occurred. Such
  /// errors can be found in Lex.errors() container.
  template<class CT, class Ty>
  static bool traverse(Ty &Dest, Lexer &Lex) {
    Token Last;
    if (Lex.is(Token::LEFT_BRACKET))
      Last = Token::RIGHT_BRACKET;
    else if (Lex.checkSpecial(Token::LEFT_BRACE))
      Last = Token::RIGHT_BRACE;
    else
      return false;
    Position Count = 0;
    if (!Lex.goToNext())
      return false;
    // It may be empty container, so it is checked here.
    // This is not the end of string so do not check that
    // mLex.next() >= mLex.json().size().
    if (Lex.is(Last))
      return true;
    for (;;) {
      std::pair<Position, Position> Key(0, Count++);
      if (Last == Token::RIGHT_BRACE) {
        if (!Lex.checkIdentifier())
          return false;
        Key = std::make_pair(Lex.start(), Lex.end());
        if (!Lex.goToNext() || !Lex.checkSpecial(Token::COLON) ||
            !Lex.goToNext())
          return false;
      }
      if (!Lex.is(Token::LEFT_BRACE) && !Lex.is(Token::LEFT_BRACKET) &&
          !Lex.checkValue())
        return false;
      if (!CT::parse(Dest, Lex, Key))
        return false;
      if (!Lex.goToNext())
        return false;
      // This is not the end of string so do not check that
      // mLex.next() >= mLex.json().size().
      if (Lex.is(Last))
        return true;
      if (!Lex.checkSpecial(Token::COMMA))
        return false;
      if (!Lex.goToNext())
        return false;
    }
  }

  /// \brief Determines number of keys in array representation.
  ///
  /// This is a utility method.
  /// Array is represented using the following JSON strings:
  /// {"0":V0, ..., "N":VN} or [V0, ..., VN].
  /// \returns At first, a number of indexes secondly the maximum index occurred
  /// in array representation and the last one value identifies whether some
  /// errors have been occurred (false if any error has been occurred).
  /// For empty arrays the second value in the tuple id undefined.
  static std::tuple<Position, Position, bool> numberOfKeys(Lexer &Lex);

  /// \brief Unparses a specified JSON object to a JSON string.
  ///
  /// NameKey parameter is a key for a field which marks JSON object identifier
  /// in a JSON string.
  static String unparse(const Object &Obj, const char *NameKey = "name") {
    assert(NameKey && "Identifier of a JSON object must not be null!");
    UnparseFunctor F(Obj, NameKey);
    ObjectTypeList::for_each_type(F);
    return F.getString();
  }

  /// \brief Unparses a specified JSON object to a JSON string.
  ///
  /// The specified JSON object will be previously converted to json::Object
  /// and then it will be unparsed. This is convenient to add identifier of
  /// the object (NameKey) to a JSON string. This identifier will be
  /// automatically added for json::Object only.
  ///
  /// NameKey parameter is a key for a field which marks JSON object identifier
  /// in a JSON string.
  template<class Ty>
  static String unparseAsObject(const Ty &Obj, const char *NameKey = "name") {
    return unparse(static_cast<const Object &>(Obj), NameKey);
  }

  /// Unparses a specified JSON object to a JSON string.
  template<class Ty,
    class = typename std::enable_if<
      !std::is_same<typename std::decay<Ty>::type, Object>::value>::type>
    static String unparse(const Ty &Obj) {
    return UnparseFunctor::unparse(Obj);
  }

  /// \brief Constructs a lexer for a specified JSON string.
  ///
  /// NameKey parameter is a key for a field which marks JSON object identifier
  /// in a JSON string.
  explicit Parser(const String &JSON, const char *NameKey = "name")
    : mLex(JSON), mNameKey(NameKey) {
    assert(NameKey && "Identifier of a JSON object must not be null!");
  }

  /// Parses JSON string and returns appropriate JSON object or
  /// nullptr if errors have occurred.
  std::unique_ptr<Object> parse() {
    if (!parseName())
      return nullptr;
    ParseFunctor F(
      mLex.json().substr(mNameStart + 1, mNameEnd - mNameStart - 1), mLex);
    ObjectTypeList::for_each_type(F);
    return F.stealObject();
  }

  /// Parses JSON string and converts it to a specified type,
  /// returns true on success.
  template<class Ty> bool parse(Ty &Obj) {
    return ParseFunctor::parse(Obj, mLex);
  }

  /// Key stored in a JSON string which marks identifier of an JSON object.
  const char *getNameKey() noexcept {
    return mNameKey;
  }

  /// Returns container of errors.
  const bcl::Diagnostic & errors() const noexcept { return mLex.errors(); }

  /// Returns true if errors have been occurred, internal errors are
  /// also considered.
  bool hasErrors() const { return mLex.hasErrors(); }

private:
  /// \brief Determines identifier to build appropriate JSONObject object.
  ///
  /// \post If this method returns true identifier is represented by characters
  /// in the range [mNameStart, mNameEnd].
  /// \return True in success, false if some errors have been occurred.
  bool parseName() {
    mLex.resetPosition();
    if (!mLex.goToNext() || !mLex.checkSpecial(Token::LEFT_BRACE))
      return false;
    while (mLex.goToNext()) {
      if (!mLex.is(Token::IDENTIFIER))
        return false;
      bool IsName = (mLex.json().compare(
        mLex.start() + 1, mLex.end() - mLex.start() - 1, getNameKey()) == 0);
      if (!mLex.goToNext() || !mLex.checkSpecial(Token::COLON) ||
          !mLex.goToNext())
        return false;
      if (IsName) {
        if (!mLex.checkIdentifier())
          return false;
        mNameStart = mLex.start();
        mNameEnd = mLex.end();
        return true;
      }
      if (((mLex.is(Token::LEFT_BRACE) || mLex.is(Token::LEFT_BRACKET)) &&
           !mLex.skipInternal()) ||
          !mLex.checkValue())
        return false;
      if (!mLex.goToNext())
        return false;
      if (mLex.is(Token::RIGHT_BRACE) && mLex.next() < mLex.json().size())
        break;
      if (!mLex.checkSpecial(Token::COMMA))
        return false;
      }
    mLex.errors().insert(JSON_ERROR(3), mLex.json().size(), getNameKey());
    return false;
  }
private:
  Lexer mLex;
  Position mNameStart = 0;
  Position mNameEnd = 0;
  const char *mNameKey;
};

template<> struct Traits<std::string> {
  inline static String unescape(const String &Str) {
    if (Str.size() < 2)
      return Str;
    std::string Res;
    Res.reserve(Str.length());
    auto I = Str.begin(), EI = Str.end() - 1;
    while (I != EI) {
      if (*I != '\\') {
        Res += *I;
        ++I;
        continue;
      }
      switch (*(++I)) {
        case 'n': Res += "\n"; break;
        case 't': Res += "\t"; break;
        case 'v': Res += "\v"; break;
        case 'f': Res += "\f"; break;
        case 'r': Res += "\r"; break;
        case '"': case '\\': Res += *I; break;
        default: continue;
      }
      ++I;
    }
    if (I == EI)
      Res += Str.back();
    return Res;
  }
  inline static Position escape(String &JSON, Position Pos) {
    switch (JSON[Pos]) {
      case '\n': JSON.replace(Pos, 1, "n"); break;
      case '\t': JSON.replace(Pos, 1, "t"); break;
      case '\v': JSON.replace(Pos, 1, "v"); break;
      case '\f': JSON.replace(Pos, 1, "f"); break;
      case '\r': JSON.replace(Pos, 1, "r"); break;
      case '"': case '\\': break;
      default: return Pos;
    }
    JSON.insert(Pos, "\\");
    return Pos + 1;
  }
  inline static bool parse(std::string &Dest, Lexer &Lex) noexcept {
    try {
      auto Value = Lex.discardQuote();
      Dest = unescape(
        Lex.json().substr(Value.first, Value.second - Value.first + 1));
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, const std::string &Obj) {
    auto I = JSON.size() + 1;
    JSON += '"' + Obj + '"';
    for (; I < JSON.size() - 1; ++I)
      I = escape(JSON, I);
  }
};

template<> struct Traits<char> {
  inline static bool parse(char &Dest, Lexer &Lex) noexcept {
    auto Value = Lex.discardQuote();
    auto Tmp = Traits<std::string>::unescape(
      Lex.json().substr(Value.first, Value.second - Value.first + 1));
    if (Tmp.size() != 1)
      return false;
    Dest = Lex.json()[Tmp.front()];
    return true;
  }
  inline static void unparse(String &JSON, char Obj) {
    auto I = JSON.size() + 1;
    JSON += '"' + Obj + '"';
    Traits<std::string>::escape(JSON, I);
  }
};

template<> struct Traits<int> {
  inline static bool parse(int &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = std::stoi(Str);
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, int Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<long> {
  inline static bool parse(long &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = std::stol(Str);
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, long Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<long long> {
  inline static bool parse(long long &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = std::stoll(Str);
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, long long Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<unsigned> {
  inline static bool parse(unsigned &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = static_cast<unsigned>(std::stoul(Str));
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, unsigned Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<unsigned long> {
  inline static bool parse(unsigned long &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = std::stoul(Str);
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, unsigned long Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<unsigned long long> {
  inline static bool parse(unsigned long long &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = std::stoull(Str);
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, unsigned long long Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<float> {
  inline static bool parse(float &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = std::stof(Str);
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, float Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<double> {
  inline static bool parse(double &Dest, Lexer &Lex) noexcept {
    try {
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = std::stod(Str);
    }
    catch (...) {
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, double Obj) {
    JSON += std::to_string(Obj);
  }
};

template<> struct Traits<bool> {
  inline static bool parse(bool &Dest, Lexer &Lex) noexcept {
    try {
      if (Lex.isKeyword(Keyword::TRUE)) {
        return Dest = true;
      } else if (Lex.isKeyword(Keyword::FALSE)) {
        return !(Dest = false);
      }
      std::string Str;
      Traits<std::string>::parse(Str, Lex);
      Dest = Str == toString(Keyword::TRUE);
      return (Str == toString(Keyword::TRUE) ||
              Str == toString(Keyword::FALSE));
    }
    catch (...) {
      return false;
    }
  }
  inline static void unparse(String &JSON, bool Obj) {
    JSON += Obj ? toString(Keyword::TRUE) : toString(Keyword::FALSE);
  }
};

template<> struct Traits<char *> {
  inline static bool parse(char *&Dest, Lexer &Lex) {
    char *TmpDest = nullptr;
    if (Lex.is(Token::LEFT_BRACE) || Lex.is(Token::LEFT_BRACKET)) {
      try {
        Position MaxIdx, Count;
        bool Ok;
        std::tie(Count, MaxIdx, Ok) = Parser<>::numberOfKeys(Lex);
        if (!Ok)
          return false;
        if (Count != 0) {
          if (Count < MaxIdx + 1) {
            /// User of JSON serializer can not determine if there is some
            /// uninitialized elements in an array without manual parsing of
            /// a JSON string. So do not parse such arrays. This array
            /// should be parsed as a map.
            Lex.errors().insert(JSON_ERROR(7), Lex.start());
            throw 0;
          } else if (Count > MaxIdx + 1) {
            Lex.errors().insert(JSON_ERROR(8), Lex.start());

            throw 0;
          }
          TmpDest = new char[MaxIdx + 1];
        }
        // Note, in case of empty array traverse also should be called to move
        // lexer position to the end of this array.
        if (!Parser<>::traverse<Traits<char *>>(TmpDest, Lex))
          throw 0;
      }
      catch (...) {
        if (TmpDest) { try { delete TmpDest; } catch (...) {} }
        return false;
      }
    } else {
      try {
        auto Value = Lex.discardQuote();
        auto Unescaped = Traits<std::string>::unescape(
          Lex.json().substr(Value.first, Value.second - Value.first + 1));
        TmpDest = new char[Unescaped.length() + 1];
        Unescaped.copy(TmpDest, Unescaped.length());
        TmpDest[Unescaped.length()] = '\0';
      }
      catch (...) {
        if (TmpDest) { try { delete TmpDest; } catch (...) {} }
        return false;
      }
    }
    Dest = TmpDest;
    return true;
  }
  inline static bool parse(char *&Dest, Lexer &Lex,
      std::pair<Position, Position> Key) {
    if (Key.first != 0) {
      // Note that Idx will be successfully converted because this check has
      // been performed when memory for the Dest array has been allocated.
      // This also implies that Dest[Idx] can not produce out of range exception.
      auto Idx = std::stoull(
        Lex.json().substr(Key.first + 1, Key.second - Key.first - 1));
      return Traits<char>::parse(Dest[Idx], Lex);
    } else {
      return Traits<char>::parse(Dest[Key.second], Lex);
    }
  }
  inline static void unparse(String &JSON, const char *Obj) {
    if (!Obj)
      return;
    auto I = JSON.size() + 1;
    JSON += "\"";
    JSON += Obj;
    JSON += "\"";
    for (; I < JSON.size() - 1; ++I)
      I = Traits<std::string>::escape(JSON, I);
  }
};

template<class Ty> struct Traits<Ty *> {
  inline static bool parse(Ty *&Dest, Lexer &Lex) {
    Ty *TmpDest = nullptr;
    if (Lex.is(Token::LEFT_BRACE) || Lex.is(Token::LEFT_BRACKET)) {
      try {
        Position MaxIdx, Count;
        bool Ok;
        std::tie(Count, MaxIdx, Ok) = Parser<>::numberOfKeys(Lex);
        if (!Ok)
          return false;
        if (Count != 0) {
          if (Count < MaxIdx + 1) {
            /// User of JSON serializer can not determine if there is some
            /// uninitialized elements in an array without manual parsing of
            /// a JSON string. So do not parse such arrays. This array
            /// should be parsed as a map.
            Lex.errors().insert(JSON_ERROR(7), Lex.start());
            throw 0;
          } else if (Count > MaxIdx + 1) {
            Lex.errors().insert(JSON_ERROR(8), Lex.start());

            throw 0;
          }
          TmpDest = new Ty[MaxIdx + 1];
        }
        // Note, in case of empty array traverse also should be called to move
        // lexer position to the end of this array.
        if (!Parser<>::traverse<Traits<Ty *>>(TmpDest, Lex))
          throw 0;
      }
      catch (...) {
        if (TmpDest) { try { delete TmpDest; } catch (...) {} }
        return false;
      }
    } else {
      try {
        auto Value = Lex.discardQuote();
        TmpDest = new Ty;
        if (!Traits<Ty>::parse(*TmpDest, Lex))
          throw 0;
      }
      catch (...) {
        if (TmpDest) { try { delete TmpDest; } catch (...) {} }
        return false;
      }
    }
    Dest = TmpDest;
    return true;
  }
  inline static bool parse(Ty *&Dest, Lexer &Lex,
      std::pair<Position, Position> Key) {
    if (Key.first != 0) {
      // Note that Idx will be successfully converted because this check has
      // been performed when memory for the Dest array has been allocated.
      // This also implies that Dest[Idx] can not produce out of range exception.
      auto Idx = std::stoull(
        Lex.json().substr(Key.first + 1, Key.second - Key.first - 1));
      return Traits<Ty>::parse(Dest[Idx], Lex);
    } else {
      return Traits<Ty>::parse(Dest[Key.second], Lex);
    }
  }
  inline static void unparse(String &JSON, const Ty* Obj) {
    if (Obj)
      Traits<Ty>::unparse(JSON, *Obj);
  }
};

template<class Ty> struct Traits<const Ty *> {
  inline static bool parse(const Ty *&Dest, Lexer &Lex) {
    Ty *TmpDest;
    if (Traits<Ty *>::parse(TmpDest, Lex)) {
      Dest = TmpDest;
      return true;
    }
    return false;
  }
  inline static void unparse(String &JSON, const Ty* Obj) {
      Traits<Ty *>::unparse(JSON, Obj);
  }
};

template<class Ty, class Allocator>
struct Traits<std::vector<Ty, Allocator>> {
  inline static bool parse(std::vector<Ty, Allocator> &Dest, Lexer &Lex) {
    Position MaxIdx, Count;
    bool Ok;
    std::tie(Count, MaxIdx, Ok) = Parser<>::numberOfKeys(Lex);
    if (!Ok)
      return false;
    if (Count != 0) {
      if (Count < MaxIdx + 1) {
        /// User of JSON serializer can not determine if there is some
        /// uninitialized elements in an array without manual parsing of
        /// a JSON string. So do not parse such arrays. This array
        /// should be parsed as a map.
        Lex.errors().insert(JSON_ERROR(7), Lex.start());
        return false;
      } else if (Count > MaxIdx + 1) {
        Lex.errors().insert(JSON_ERROR(8), Lex.start());
        return false;
      }
    }
    Dest.resize(Count);
    // Note, in case of empty array traverse also should be called to move
    // lexer position to the end of this array.
    return Parser<>::
      traverse<Traits<std::vector<Ty, Allocator>>>(Dest, Lex);
  }
  inline static bool parse(std::vector<Ty, Allocator> &Dest, Lexer &Lex,
      std::pair<Position, Position> Key) {
    if (Key.first != 0) {
      // Note that Idx will be successfully converted because this check has
      // been performed when memory for the Dest array has been allocated.
      // This also implies that Dest[Idx] can not produce out of range exception.
      auto Idx = std::stoull(
        Lex.json().substr(Key.first + 1, Key.second - Key.first - 1));
      return Traits<Ty>::parse(Dest[Idx], Lex);
    } else {
      return Traits<Ty>::parse(Dest[Key.second], Lex);
    }
  }
  inline static void unparse(String &JSON,
      const std::vector<Ty, Allocator> &Obj) {
    typedef std::vector<Ty, Allocator> VecTy;
    if (!Obj.empty()) {
      std::vector<String> Values(Obj.size());
      bool HasEmpty = false;
      for (typename VecTy::size_type I = 0; I < Obj.size(); ++I) {
        Traits<typename VecTy::value_type>::unparse(Values[I], Obj[I]);
        HasEmpty = HasEmpty || Values[I].empty();
      }
      if (HasEmpty) {
        JSON += '{';
        for (typename VecTy::size_type I = 0; I < Obj.size(); ++I) {
          if (Values[I].empty())
            continue;
          JSON += "\"";
          Traits<unsigned long long>::unparse(JSON, I);
          JSON += "\":" + Values[I] + ",";
        }
        assert(JSON.back() == ',' && "A delimiter must be a comma!");
        JSON.erase(JSON.size() - 1);
        JSON += '}';
      } else {
        JSON += '[';
        for (auto &V : Values)
          JSON += V + ',';
        assert(JSON.back() == ',' && "A delimiter must be a comma!");
        JSON.erase(JSON.size() - 1);
        JSON += ']';
      }
    } else {
      JSON += "[]";
    }
  }
};

template<class KeyTy, class Compare, class Allocator>
struct Traits<std::set<KeyTy, Compare, Allocator>> {
  typedef std::set<KeyTy, Compare, Allocator> SetTy;
  inline static bool parse(SetTy &Dest, Lexer &Lex) {
    if (!Lex.checkSpecial(Token::LEFT_BRACKET))
      return false;
    return Parser<>::traverse<Traits<SetTy>>(Dest, Lex);
  }
  inline static bool parse(SetTy &Dest, Lexer &Lex,
      std::pair<Position, Position> Key) {
    KeyTy KeyValue;
    if (!Traits<KeyTy>::parse(KeyValue, Lex))
      return false;
    auto Pair = Dest.insert(std::move(KeyValue));
    if (!Pair.second) {
      Lex.errors().insert(JSON_ERROR(8), Lex.start());
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, const SetTy &Obj) {
    JSON += '[';
    if (!Obj.empty()) {
      auto I = Obj.begin(), EI = Obj.end();
      for (--EI; I != EI; ++I) {
        String Key;
        Traits<KeyTy>::unparse(Key, *I);
        if (!Key.empty())
          JSON += Key + ',';
      }
      String Key;
      Traits<KeyTy>::unparse(Key, *I);
      if (!Key.empty())
        JSON += Key;
      else if (JSON.back() == ',')
        JSON.erase(JSON.size() - 1);
    }
    JSON += ']';
  }
};

template<class KeyTy, class Ty, class Compare, class Allocator>
struct Traits<std::map<KeyTy, Ty, Compare, Allocator>> {
  typedef std::map<KeyTy, Ty, Compare, Allocator> MapTy;
  inline static bool parse(MapTy &Dest, Lexer &Lex) {
    return Parser<>::traverse<Traits<MapTy>>(Dest, Lex);
  }
  inline static bool parse(MapTy &Dest, Lexer &Lex,
      std::pair<Position, Position> Key) {
    Lex.storePosition();
    Lex.setPosition(Key.first);
    KeyTy KeyValue;
    Traits<KeyTy>::parse(KeyValue, Lex);
    Lex.restorePosition();
    auto Pair = Dest.emplace(KeyValue, Ty());
    if (!Pair.second) {
      Lex.errors().insert(JSON_ERROR(8), Lex.start());
      return false;
    }
    if (!Traits<Ty>::parse(Pair.first->second, Lex)) {
      Dest.erase(Pair.first);
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, const MapTy &Obj) {
    JSON += '{';
    if (!Obj.empty()) {
      auto I = Obj.begin(), EI = Obj.end();
      for (--EI; I != EI; ++I) {
        String Value;
        Traits<typename MapTy::mapped_type>::unparse(Value, I->second);
        if (!Value.empty()) {
          String Id;
          Traits<typename MapTy::key_type>::unparse(Id, I->first);
          Id = bcl::quote(std::move(Id));
          JSON += Id + ':'+ Value + ',';
        }
      }
      String Value;
      Traits<typename MapTy::mapped_type>::unparse(Value, I->second);
      if (!Value.empty()) {
        String Id;
        Traits<typename MapTy::key_type>::unparse(Id, I->first);
        Id = bcl::quote(std::move(Id));
        JSON += Id + ':' + Value;
      } else if (JSON.back() == ',') {
        JSON.erase(JSON.size() - 1);
      }
    }
    JSON += '}';
  }
};

template<class KeyTy, class Ty, class Compare, class Allocator>
struct Traits<std::multimap<KeyTy, Ty, Compare, Allocator>> {
  typedef std::multimap<KeyTy, Ty, Compare, Allocator> MapTy;
  inline static bool parse(MapTy &Dest, Lexer &Lex) {
    return Parser<>::traverse<Traits<MapTy>>(Dest, Lex);
  }
  inline static bool parse(MapTy &Dest, Lexer &Lex,
    std::pair<Position, Position> Key) {
    Lex.storePosition();
    Lex.setPosition(Key.first);
    KeyTy KeyValue;
    Traits<KeyTy>::parse(KeyValue, Lex);
    Lex.restorePosition();
    auto Itr = Dest.emplace(KeyValue, Ty());
    if (Itr == Dest.end())
      return false;
    if (!Traits<Ty>::parse(Itr->second, Lex)) {
      Dest.erase(Itr);
      return false;
    }
    return true;
  }
  inline static void unparse(String &JSON, const MapTy &Obj) {
    JSON += '{';
    if (!Obj.empty()) {
      auto I = Obj.begin(), EI = Obj.end();
      for (--EI; I != EI; ++I) {
        String Value;
        Traits<typename MapTy::mapped_type>::unparse(Value, I->second);
        if (!Value.empty()) {
          String Id;
          Traits<typename MapTy::key_type>::unparse(Id, I->first);
          Id = bcl::quote(std::move(Id));
          JSON += Id + ':' + Value + ',';
        }
      }
      String Value;
      Traits<typename MapTy::mapped_type>::unparse(Value, I->second);
      if (!Value.empty()) {
        String Id;
        Traits<typename MapTy::key_type>::unparse(Id, I->first);
        Id = bcl::quote(std::move(Id));
        JSON += Id + ':' + Value;
      } else if (JSON.back() == ',') {
        JSON.erase(JSON.size() - 1);
      }
    }
    JSON += '}';
  }
};

namespace detail {
/// This functor parses a value which starts with the last token extracted
/// from a JSON string, converts it to an appropriate type and assigns
/// to an appropriate field in a JSON object implemented as a bcl::StaticMap.
class ParseCellFunctor {
public:
  /// \brief Creates this functor to parse a value associated with
  /// a specified key.
  ///
  /// The Key parameter is necessary to identify destination which should be set
  /// on the parsed value.
  ParseCellFunctor(std::pair<Position, Position> Key, Lexer &Lex) : mLex(Lex),
    mKey(Key), mHasError(false) {}

  /// \brief Parses a value specified by the last token evaluated by the
  /// lexer, converts it to an appropriate type and assigns to a currently
  /// evaluated key determined in constructor.
  ///
  /// Note that the current token may be beginning of a compound value,
  /// for example '{'.
  template<class CellTy> void operator()(CellTy *Cell) {
    typedef typename CellTy::CellKey CellKey;
    if (mLex.json().compare(
          mKey.first + 1, mKey.second - mKey.first - 1 ,
          CellTraits<CellKey>::name()) != 0)
      return;
    mHasError = !CellTraits<CellKey>::parse(
      Cell->template value<CellKey>(), mLex);
  }

  /// Returns true if errors have been occurred during conversion.
  bool hasError() const noexcept { return mHasError; }
private:
  Lexer &mLex;
  std::pair<Position, Position> mKey;
  bool mHasError;
};

/// This functor unparses JSON object represented as a bcl::StaticMap. It should
/// be called for each cell in the map.
class UnparseCellFunctor {
public:
  /// Creates functor which stores unparsed data in a specified JSON string.
  explicit UnparseCellFunctor(String &JSON) : mIsFirst(true), mJSON(JSON) {}

  /// Unparses a specified cell if it has a value.
  template<class CellTy> void operator()(CellTy *Cell) {
    typedef typename CellTy::CellKey CellKey;
    String Value;
    CellTraits<CellKey>::unparse(Value, Cell->template value<CellKey>());
    if (Value.empty())
      return;
    if (!mIsFirst)
      mJSON += ',';
    else
      mIsFirst = false;
    Traits<
      typename std::decay<
        typename std::result_of<
          decltype(&CellTraits<CellKey>::name)()>::type>::type>::
      unparse(mJSON, CellTraits<CellKey>::name());
    mJSON += ':' + Value;
  }
private:
  bool mIsFirst;
  String &mJSON;
};
}

template<class... Args> struct Traits<bcl::StaticMap<Args...>> {
  inline static bool parse(bcl::StaticMap<Args...> &Dest, Lexer &Lex) {
    if (!Lex.checkSpecial(Token::LEFT_BRACE))
      return false;
    return Parser<>::
      traverse<Traits<bcl::StaticMap<Args...>>>(Dest, Lex);
  }
  inline static bool parse(bcl::StaticMap<Args...> &Dest, Lexer &Lex,
      std::pair<Position, Position> Key) {
    detail::ParseCellFunctor Parse(Key, Lex);
    Dest.for_each(Parse);
    return !Parse.hasError();
  }
  inline static void unparse(String &JSON, const bcl::StaticMap<Args...> &Obj) {
    detail::UnparseCellFunctor Unparse(JSON);
    JSON += '{';
    Obj.for_each(Unparse);
    JSON += '}';
  }
};

template<class... Objects> std::tuple<Position, Position, bool>
Parser<Objects...>::numberOfKeys(Lexer &Lex) {
  Lex.storePosition();
  Position MaxIdx = 0;
  Position Count = 0;
  Token Last;
  if (Lex.is(Token::LEFT_BRACKET))
    Last = Token::RIGHT_BRACKET;
  else if (Lex.checkSpecial(Token::LEFT_BRACE))
    Last = Token::RIGHT_BRACE;
  else
    return std::make_tuple(0, 0, false);
  if (!Lex.goToNext())
    return std::make_tuple(0, 0, false);
  if (Lex.is(Last)) {
    Lex.restorePosition();
    return std::make_tuple(0, 0, true);
  }
  for (;;) {
    ++Count;
    if (Last == Token::RIGHT_BRACE) {
      if (!Lex.checkIdentifier())
        return std::make_tuple(0, 0, false);
      unsigned long long Idx;
      if (!Traits<unsigned long long>::parse(Idx, Lex))
        return std::make_tuple(0, 0, false);
      MaxIdx = static_cast<Position>(Idx > MaxIdx ? Idx : MaxIdx);
      if (!Lex.goToNext() || !Lex.checkSpecial(Token::COLON) ||
          !Lex.goToNext())
        return std::make_tuple(0, 0, false);
    }
    if (Lex.is(Token::LEFT_BRACE) || Lex.is(Token::LEFT_BRACKET)) {
      if (!Lex.skipInternal())
        return std::make_tuple(0, 0, false);
    } else if (!Lex.checkValue()) {
      return std::make_tuple(0, 0, false);
    }
    if (!Lex.goToNext())
      return std::make_tuple(0, 0, false);
    if (Lex.is(Last))
      break;
    if (!Lex.checkSpecial(Token::COMMA))
      return std::make_tuple(0, 0, false);
    if (!Lex.goToNext())
      return std::make_tuple(0, 0, false);
  }
  Lex.restorePosition();
  return std::make_tuple(
    Count, Last == Token::RIGHT_BRACE ? MaxIdx : Count - 1, true);
}

template<> struct Traits<bcl::Diagnostic> {
  inline static bool parse(bcl::Diagnostic &Dest, Lexer &Lex) {
    return Parser<>::
      traverse<Traits<bcl::Diagnostic>>(Dest, Lex);
  }
  inline static bool parse(bcl::Diagnostic &Dest, Lexer &Lex,
    std::pair<Position, Position>) {
    const char *Msg;
    if (!Traits<const char *>::parse(Msg, Lex))
      return false;
    char *Kind = new char[std::strlen(Msg) + 1];
    char *Buf = new char[std::strlen(Msg) + 1];
    std::size_t Code;
    std::uintmax_t Pos;
    auto ArgNum = std::sscanf(Msg, "%s C%zu(%ju): %s", Kind, &Code, &Pos, Buf);
    if (ArgNum != 4 || std::strcmp(Kind, Dest.getKind()) != 0 ||
        !Dest.insert(Code, "%s", Pos, Buf)) {
      Lex.errors().insert(JSON_ERROR(9), Lex.start());
      delete[]Kind;
      delete[]Buf;
      return false;
    }
    delete[]Kind;
    delete[]Buf;
    return true;
  }
  inline static void unparse(String &JSON, const bcl::Diagnostic &Obj) {
    JSON += '[';
    if (!Obj.empty()) {
      auto I = Obj.begin(), EI = Obj.end();
      bcl::Diagnostic::size_type Idx = 0;
      for (--EI; I != EI; ++I, ++Idx) {
        Traits<bcl::Diagnostic::value_type>::unparse(JSON, *I);
        JSON += ',';
      }
      Traits<bcl::Diagnostic::value_type>::unparse(JSON, *I);
    }
    JSON += ']';
  }
};

/// Specialization of JSON serialization traits for std::optional type.
///
/// Note that 'T' must be default constructible and copy assignable.
template<class T> struct Traits<std::optional<T>> {
  static_assert(std::is_default_constructible<T>::value,
    "Underlining type must be default constructible!");
  static_assert(std::is_copy_assignable<T>::value,
    "Underlining type must be copy assignable!");
  static bool parse(std::optional<T> &Dest, ::json::Lexer &Lex) noexcept {
    try {
      std::string Str;
      if (Lex.isKeyword(Keyword::NO_VALUE) ||
          Traits<std::string>::parse(Str, Lex) &&
              Str == toString(Keyword::NO_VALUE)) {
        Dest.reset();
      } else {
        T Tmp;
        Traits<T>::parse(Tmp, Lex);
        Dest = std::move(Tmp);
      }
    }
    catch (...) {
      return false;
    }
    return true;
  }
  static void unparse(String &JSON, const std::optional<T> &Obj) {
    if (Obj.has_value())
      Traits<T>::unparse(JSON, *Obj);
    else
      JSON += toString(Keyword::NO_VALUE);
  }
};
}

//===- Definition of macros which simplifies definition of a JSON-object --===//
#include "JsonObjectMacros.h"

#endif// TSAR_REQUESTS_H

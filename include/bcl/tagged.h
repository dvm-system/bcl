//===--- tagged.h --------------- Type Tag Idiom ----------------*- C++ -*-===//
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
// In some cases it is convenient to tag a type. For example, tags (or keys) are
// used to access elements in bcl::StaticMap.
// \code
//   struct Name {typedf std::string ValueType;};
//   sturct Age {typedef unsigned ValueType;};
//
//   bcl::StaticMap<Name> Person;
//   Person.value<Name>() = "Ivan";
//   Person.value<Age>() = 21;)
//   std::cout << Person.value<Name>() << " is "
//     << Person.value<Age>() << "years old\n";
// \endcode
// In this example Name is a tag for std::string and Age is a tag for unsigned.
//
// This file implements other simple way of tagging a type. The idea comes from
// Boost library but it has been extended in some ways:
//   * It is possible to use different tags for a one type.
//   * A general way to find necessary tag in a list of tags is implemented.
//   * The use of tags is simplified by facilities of C++ 11.
//   * Tagged pair is also available. It is the same as a std::pair but also
//     provides method get() to access its values as Pair.get<Tag>().
//
// To tag a type use bcl::tagged<...> template.
//===----------------------------------------------------------------------===//

#ifndef TAGGED_H
#define TAGGED_H

#include "cell.h"
#include "utility.h"
#include <tuple>
#include <utility>

namespace bcl {
/// \brief This is used to tag a type Ty. The main tag is TagTy, but there are
/// also alias tags which are specified in AliasTy template parameter.
///
/// Type aliases tagged_t<...>, tagged_tag<...>, tagged_alias<...> are also
/// available to access appropriate typedefs.
template<class Ty, class TagTy, class... AliasTy>
struct tagged {
  typedef Ty type;
  typedef TagTy tag;
  typedef TypeList<TagTy, AliasTy...> alias;
};

/// \brief Checks if Ty is a bcl::tagged type and provides the member constant
/// `value` equal to true. Otherwise `value` is false.
template<class Ty>
struct is_tagged : public std::false_type {};

/// \brief Checks if Ty is a bcl::tagged type and provides the member constant
/// `value` equal to true. Otherwise `value` is false.
template<class... Args>
struct is_tagged<tagged<Args...>> : public std::true_type {};

/// Specify additional functionality to tag a type.
namespace tags {

/// \brief Make alias for a tag.
///
/// There are two possible way to use this structure:
/// - add_alias<tagged<...>, Args...> defines a new tagged structure and joins
/// list of tags from the specified bcl::tagged type and list of tags (Args...).
/// - add_alias<Ty, TagTy, AliasTy...> is the same as a
/// bcl::tagged<Ty, TagTy, AliasTy>.
///
/// To access the defined tags this provides a typedef `type`. It is possible to
/// use bcl::add_alias_tagged<...> type alias to access it.
template<class... Args> struct add_alias {
  typedef tagged<Args...> type;
};

/// Make alias for a tag.
template<class... Args, class... NewAliasTy>
struct add_alias<tagged<Args...>, NewAliasTy...> {
  typedef tagged<Args..., NewAliasTy...> type;
};


/// Make alias for a tag.
template <class... Args> struct add_alias_list;

/// Make alias for a tag.
template<class... Args, class... NewAliasTy>
struct add_alias_list<tagged<Args...>, TypeList<NewAliasTy...>> {
  typedef tagged<Args..., NewAliasTy...> type;
};

/// \brief Checks if TagTy is a tag for Tagged::type.
///
/// If TagTy is a tag the member constant `value` equal to true.
/// Otherwise value is false.
template<class TagTy, class Tagged> struct is_alias :
  public IsTypeExist<TagTy, typename Tagged::alias> {};

/// \brief Finds bcl::tagged (in Taggeds) structure which is associated with the
/// specified tag TagTy.
///
/// If appropriated structure has not been found it provide a typedef type which
/// is alias `void`. There is also a  specialization of this template to search
/// tags in a bcl::TapeList<Taggeds...>. It is possible to use
/// bcl::get_tagged<...> type alias to access result of search.
template<class TagTy, class... Taggeds> struct get;

template<class TagTy> struct get<TagTy> {
  typedef void type;
};

/// Finds bcl::tagged structure which is associated with the specified tag.
template<class TagTy, class Tagged, class... Taggeds>
struct get<TagTy, Tagged, Taggeds...> {
  typedef typename std::conditional<is_alias<TagTy, Tagged>::value,
    Tagged, typename get<TagTy, Taggeds...>::type>::type type;
};

/// Finds bcl::tagged structure which is associated with the specified tag.
template<class TagTy, class... Taggeds>
struct get<TagTy, TypeList<Taggeds...>> {
  typedef typename get<TagTy, Taggeds...>::type type;
};

namespace detail {
template<class Tagged> struct type_or_void {
  using type = typename Tagged::type;
};
template<> struct type_or_void<void> { using type = void; };

template<class Tagged> struct tag_or_void {
  using tag = typename Tagged::tag;
};
template<> struct tag_or_void<void> { using tag = void; };

template<class Tagged> struct alias_or_void {
  using alias = typename Tagged::alias;
};
template<> struct alias_or_void<void> { using alias = void; };
}
}

/// Type alias to access a tagged type.
template<class Ty, class TagTy, class... AliasTy>
using tagged_t = typename tagged<Ty, TagTy, AliasTy...>::type;

/// Type alias to access the main tag for a type.
template<class Ty, class TagTy, class... AliasTy>
using tagged_tag = typename tagged<Ty, TagTy, AliasTy...>::tag;

/// Type alias to access all possible tags for a type.
template<class Ty, class TagTy, class... AliasTy>
using tagged_alias = typename tagged<Ty, TagTy, AliasTy...>::alias;

/// Type alias to access result of bcl::tagged::add_alias.
template<class... Args>
using add_alias_tagged = typename tags::add_alias<Args...>::type;

/// Type alias to access result of bcl::tagged::add_alias.
template<class... Args>
using add_alias_tagged_t = typename add_alias_tagged<Args...>::type;

/// Type alias to access result of bcl::tagged::add_alias.
template<class... Args>
using add_alias_tagged_tag = typename add_alias_tagged<Args...>::tag;

/// Type alias to access result of bcl::tagged::add_alias.
template<class... Args>
using add_alias_tagged_alias = typename add_alias_tagged<Args...>::alias;

/// Type alias to access result of bcl::tagged::add_alias_list.
template <class... Args>
using add_alias_list_tagged = typename tags::add_alias_list<Args...>::type;

/// Type alias to access result of bcl::tagged::add_alias_list.
template <class... Args>
using add_alias_list_tagged_t = typename add_alias_list_tagged<Args...>::type;

/// Type alias to access result of bcl::tagged::add_alias_list.
template <class... Args>
using add_alias_list_tagged_tag = typename add_alias_list_tagged<Args...>::tag;

/// Type alias to access result of bcl::tagged::add_alias_list.
template <class... Args>
using add_alias_list_tagged_alias =
    typename add_alias_list_tagged<Args...>::alias;

/// Type alias to access result of search an appropriate bcl::tagged structure.
template<class TagTy, class... Taggeds>
using get_tagged = typename tags::get<TagTy, Taggeds...>::type;

/// Type alias to access result of search an appropriate bcl::tagged structure.
template<class TagTy, class... Taggeds>
using get_tagged_t = typename tags::detail::type_or_void<
  get_tagged<TagTy, Taggeds...>>::type;

/// Type alias to access result of search an appropriate bcl::tagged structure.
template<class TagTy, class... Taggeds>
using get_tagged_tag = typename tags::detail::tag_or_void<
  get_tagged<TagTy, Taggeds...>>::tag;

/// Type alias to access result of search an appropriate bcl::tagged structure.
template<class TagTy, class... Taggeds>
using get_tagged_alias = typename tags::detail::alias_or_void<
  get_tagged<TagTy, Taggeds...>>::alias;

/// This is equivalent to std::pair but also it provides way to access first and
/// second value via tag of a type (Pair.get<Tag>()).
template<class Tagged1, class Tagged2>
struct tagged_pair :
    public std::pair<typename Tagged1::type, typename Tagged2::type> {
  using taggeds = bcl::TypeList<Tagged1, Tagged2>;

  template<class... ArgsTy,
    class = typename std::enable_if<
      std::is_constructible<
        std::pair<typename Tagged1::type, typename Tagged2::type>,
          ArgsTy&&...>::value>::type>
  constexpr tagged_pair(ArgsTy&&... Args) :
    std::pair<typename Tagged1::type, typename Tagged2::type>(
      std::forward<ArgsTy>(Args)...) {}

  template<class ArgTy,
    class = typename std::enable_if<
      std::is_constructible<
        std::pair<typename Tagged1::type, typename Tagged2::type>,
          ArgTy &&>::value>::type>
  tagged_pair & operator=(ArgTy&& Arg)
  /// TODO (kaniandr@gmail.com: is it possible to use noexcept here
  /// (see description of this issue in convertible_pair.h).
#ifdef _MSC_VER
    noexcept(noexcept(
      std::pair<typename Tagged1::type, typename Tagged2::type>::
        operator=(std::forward<ArgTy>(Arg))))
#endif
  {
    return static_cast<tagged_pair &>(
      std::pair<typename Tagged1::type, typename Tagged2::type>::
        operator=(std::forward<ArgTy>(Arg)));
  }

  template<class Tag,
  class = typename std::enable_if<
    !std::is_void<bcl::get_tagged<Tag, Tagged1, Tagged2>>::value>::type>
  bcl::get_tagged_t<Tag, Tagged1, Tagged2> & get() noexcept {
    return get(
      std::is_same<Tagged1, bcl::get_tagged<Tag, Tagged1, Tagged2>>());
  }

  template<class Tag,
  class = typename std::enable_if<
    !std::is_void<bcl::get_tagged<Tag, Tagged1, Tagged2>>::value>::type>
  const bcl::get_tagged_t<Tag, Tagged1, Tagged2> & get() const noexcept {
    return get(
      std::is_same<Tagged1, bcl::get_tagged<Tag, Tagged1, Tagged2>>());
  }
private:
  typename Tagged1::type & get(std::true_type) noexcept {
    return std::pair<typename Tagged1::type, typename Tagged2::type>::first;
  }

  typename Tagged2::type & get(std::false_type) noexcept {
    return std::pair<typename Tagged1::type, typename Tagged2::type>::second;
  }

  const typename Tagged1::type & get(std::true_type) const noexcept {
    return std::pair<typename Tagged1::type, typename Tagged2::type>::first;
  }

  const typename Tagged2::type & get(std::false_type) const noexcept {
    return std::pair<typename Tagged1::type, typename Tagged2::type>::second;
  }
};

/// This is equivalent to std::tuple but also it provides way to access values
/// via tag of a type (Tuple.get<Tag>()).
template<class... Taggeds>
struct tagged_tuple: public std::tuple<typename Taggeds::type...> {
  using taggeds = bcl::TypeList<Taggeds...>;
  using tuple = std::tuple<typename Taggeds::type...>;

  template<class... ArgsTy,
    class = typename std::enable_if<
      std::is_constructible<
        std::tuple<typename Taggeds::type...>, ArgsTy&&...>::value>::type>
  constexpr tagged_tuple(ArgsTy&&... Args) :
    std::tuple<typename Taggeds::type...>(std::forward<ArgsTy>(Args)...) {}

  template<class... ArgsTy,
    class = typename std::enable_if<
      std::is_constructible<
        std::tuple<typename Taggeds::type...>, ArgsTy&&...>::value>::type>
  tagged_tuple & operator=(ArgsTy&&... Args) noexcept(noexcept(
    std::tuple<typename Taggeds::type...>::
      operator=(std::forward<ArgsTy>(Args)...))) {
    return static_cast<tagged_tuple &>(
      std::tuple<typename Taggeds::type...>::
        operator=(std::forward<ArgsTy>(Args)...));
  }

  template<class Tag,
    class = typename std::enable_if<
      !std::is_void<bcl::get_tagged<Tag, Taggeds...>>::value>::type>
  bcl::get_tagged_t<Tag, Taggeds...> & get() noexcept {
    return std::get<
      bcl::index_of<bcl::get_tagged<Tag, Taggeds...>, Taggeds...>()>(*this);
  }

  template<class Tag,
    class = typename std::enable_if<
      !std::is_void<bcl::get_tagged<Tag, Taggeds...>>::value>::type>
  const bcl::get_tagged_t<Tag, Taggeds...> & get() const noexcept {
    return std::get<
      bcl::index_of<bcl::get_tagged<Tag, Taggeds...>, Taggeds...>()>(*this);
  }
};

/// Provide access to the number of elements in a tuple.
template<class T> class tagged_tuple_size;

/// Provide access to the number of elements in a tuple.
template<class... Taggeds>
class tagged_tuple_size<tagged_tuple<Taggeds...>> :
  public std::tuple_size<typename tagged_tuple<Taggeds...>::tuple> {};

/// Evaluate to true if there are no elements into a tuple.
template<class T> class tagged_tuple_empty :
  public std::conditional<(tagged_tuple_size<T>::value > 0),
    std::false_type, std::true_type>::type{};

/// Provide compile-time access to the types of the elements of a tuple.
template<class Tag, class T> class tagged_tuple_element;

/// Provide compile-time access to the types of the elements of a tuple.
template<class Tag, class... Taggeds>
class tagged_tuple_element<Tag, tagged_tuple<Taggeds...>> :
  public tags::get<Tag, Taggeds...>::type {};

/// Provide compile time access to the index of a specified tag in a tuple.
template<class Tag, class T> class tagged_tuple_idx;

/// Provide compile time access to the index of a specified tag in a tuple.
template<class Tag, class... Taggeds>
class tagged_tuple_idx<Tag, tagged_tuple<Taggeds...>> :
  public std::integral_constant<
    std::size_t, bcl::index_of<Tag, Taggeds...>()> {};

/// Provide compile time access to a tag with a specified index.
template<std::size_t Idx, class T> struct tagged_tuple_tag;

/// Provide compile time access to a tag with a specified index.
template<std::size_t Idx, class First, class... Taggeds>
struct tagged_tuple_tag<Idx, tagged_tuple<First, Taggeds...>> :
  public tagged_tuple_tag<Idx - 1, tagged_tuple<Taggeds...>> {};

/// Provide compile time access to a tag with a specified index.
template<class First, class... Taggeds>
struct tagged_tuple_tag<0, tagged_tuple<First, Taggeds...>> {
  using type = typename First::tag;
};

namespace tags {
namespace detail {
template<std::size_t Idx, class Function, class... Taggeds>
void for_each(const tagged_tuple<Taggeds...> &T,
    Function &&F, std::true_type) {}

template<std::size_t Idx, class Function, class... Taggeds>
void for_each(tagged_tuple<Taggeds...> &T, Function &&F, std::false_type) {
  using TupleT = tagged_tuple<Taggeds...>;
  using TagT = typename tagged_tuple_tag<Idx, TupleT>::type;
#if defined __GNUC__ || defined __clang__ || defined _MSC_VER && _MSC_VER >= 1911
  F.template operator()<get_tagged<TagT, Taggeds...>>(
    T.TupleT:: template get<TagT>());
#else
  F.operator()<get_tagged<TagT, Taggeds...>>(T.get<TagT>());
#endif
  using IsLast = typename std::conditional<
    (Idx + 1 < tagged_tuple_size<TupleT>::value),
    std::false_type, std::true_type>::type;
  for_each<Idx + 1>(T, F, IsLast());
}

template<std::size_t Idx, class Function, class... Taggeds>
void for_each(const tagged_tuple<Taggeds...> &T, Function &&F,
    std::false_type) {
  using TupleT = tagged_tuple<Taggeds...>;
  using TagT = typename tagged_tuple_tag<Idx, TupleT>::type;
#if defined __GNUC__ || defined __clang__ || defined _MSC_VER && _MSC_VER >= 1911
  F.template operator()<get_tagged<TagT, Taggeds...>>(
    T.TupleT:: template get<TagT>());
#else
  F.operator()<get_tagged<TagT, Taggeds...>>(T.get<TagT>());
#endif
  using IsLast = typename std::conditional<
    (Idx + 1 < tagged_tuple_size<TupleT>::value),
    std::false_type, std::true_type>::type;
  for_each<Idx + 1>(T, F, IsLast());
}
}
}

/// Call a specified function for each element in a tuple.
template<class Function, class... Taggeds>
void for_each(tagged_tuple<Taggeds...> &T, Function &&F) {
  using TupleT = tagged_tuple<Taggeds...>;
  tags::detail::for_each<0>(T, std::forward<Function>(F),
    tagged_tuple_empty<TupleT>());
}

/// Call a specified function for each element in a tuple.
template<class Function, class... Taggeds>
void for_each(const tagged_tuple<Taggeds...> &T, Function &&F) {
  using TupleT = tagged_tuple<Taggeds...>;
  tags::detail::for_each<0>(T, std::forward<Function>(F),
    tagged_tuple_empty<TupleT>());
}

namespace tags {
namespace detail {
template<class Ty, class Tags, class... Taggeds>
struct get_tagged_tuple_impl {
  using type = typename get_tagged_tuple_impl<Ty, typename Tags::Next,
    tagged<Ty, typename Tags::Type>, Taggeds...>::type;
};

template<class Ty, class... Taggeds>
struct get_tagged_tuple_impl<Ty, TypeList<>, Taggeds...> {
  using type = tagged_tuple<Taggeds...>;
};
}

/// Construct bcl::tagged_tuple which stores values of a type `Ty` with tags
/// `Tags`.
template<class Ty, class... Tags>
struct get_tagged_tuple {
  using type =
    typename detail::get_tagged_tuple_impl<Ty, TypeList<Tags...>>::type;
};

/// Construct bcl::tagged_tuple which stores values of a type `Ty` with tags
/// `Tags`.
template<class Ty, class... Tags>
struct get_tagged_tuple<Ty, bcl::TypeList<Tags...>> {
  using type =
    typename detail::get_tagged_tuple_impl<Ty, TypeList<Tags...>>::type;
};

/// Type alias to access constructed bcl::tagged_tuple type.
template<class Ty, class... Tags>
using get_tagged_tuple_t = typename get_tagged_tuple<Ty, Tags...>::type;
}
}
#endif//TAGGED_H

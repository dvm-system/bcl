//===----- trait.h -------- Object Properties Collection --------*- C++ -*-===//
//
//                       Base Construction Library (BCL)
//
//===----------------------------------------------------------------------===//
//
// This file defines classes that could be useful to store different properties
// of objects. This properties are called traits which have a unique keys.
// These keys are represented as a bit flags specified by the user of a
// proposed functionality.
//
//===----------------------------------------------------------------------===//

#ifndef TRAIT_H
#define TRAIT_H

#include <climits>
#include <cell.h>
#include <utility.h>
#include <type_traits>
#include <bitset>

namespace bcl {
/// This type represents a key of a trait.
typedef unsigned long long TraitKey;

/// Base representation of a trait, all traits must inherit this class.
template<TraitKey Key> struct TraitBase {
  static constexpr TraitKey getKey() { return Key; }
};

/// Representation of a group of alternative traits.
template<class... Traits> struct TraitAlternative {};

/// Representation of a traits from group of alternative traits which can occur
/// simultaneously.
template<class... Traits> struct TraitUnion {};

/// \brief Constructs list of all traits belongs to specified groups.
///
/// Note that this does not remove duplicate traits, to remove them use
/// bcl::RemoveDuplicate<typename TraitList<...>::Type>.
template<class... Groups> struct TraitList { typedef bcl::TypeList<> Type; };

/// Constructs list of all traits belongs to specified groups.
template<class First, class... Groups> struct TraitList<First, Groups...> {
  typedef typename bcl::MergeTypeLists<
    typename TraitList<First>::Type,
    typename TraitList<Groups...>::Type>::Type Type;
};

/// Constructs list of all traits belongs to specified groups.
template<class... Traits> struct TraitList<TraitUnion<Traits...>> {
  typedef typename TraitList<Traits...>::Type Type;
};

/// Constructs list of all traits belongs to specified groups.
template<class... Traits> struct TraitList<TraitAlternative<Traits...>> {
  typedef typename TraitList<Traits...>::Type Type;
};

/// Constructs list of all traits belongs to specified groups.
template<class Trait> struct TraitList<Trait> {
  typedef bcl::TypeList<Trait> Type;
};

namespace trait {
namespace detail {
/// Joins (equivalent to |) local keys for specified traits.
template<class Descriptor, class... Traits> struct Join {};

/// Joins (equivalent to |) local keys for specified traits.
template<class Descriptor, class First, class... Traits>
struct Join<Descriptor, First, Traits...> {
  static constexpr TraitKey value() {
    return Join<Descriptor, First>::value() |
      Join<Descriptor, Traits...>::value();
  }

  template<class Ty>
  static constexpr TraitKey valueWithout() {
    return Join<Descriptor, First>::template valueWithout<Ty>() |
      Join<Descriptor, Traits...>::template valueWithout<Ty>();
  }
};

/// Joins (equivalent to |) local keys for specified traits.
template<class Descriptor, class... Traits>
struct Join<Descriptor, TypeList<Traits...>> :
  public Join<Descriptor, Traits...> {};

/// Joins (equivalent to |) local keys for specified traits.
template<class Descriptor, class... Traits>
struct Join<Descriptor, TraitUnion<Traits...>> :
  public Join<Descriptor, Traits...> {};

/// Joins (equivalent to |) local keys for specified traits.
template<class Descriptor, class... Traits>
struct Join<Descriptor, TraitAlternative<Traits...>> :
  public Join<Descriptor, Traits...> {};

/// Joins (equivalent to |) local keys for specified traits.
template<class Descriptor, class Trait>
struct Join<Descriptor, Trait> {
  static constexpr TraitKey value() { return Descriptor::template getKey<Trait>(); }

  template<class Ty>
  static constexpr TraitKey valueWithout() {
    return std::is_same<Ty, Trait>::value ? 0 : Descriptor::template getKey<Trait>();
  }
};

/// Joins (equivalent to |) local keys for specified traits.
template<class Descriptor> struct Join<Descriptor> {
  static constexpr TraitKey value() { return 0; }

  template<class Ty>
  static constexpr TraitKey valueWithout() { return 0; }
};
}

/// Joins local (equivalent to |) local keys for specified traits.
template<class Descriptor, class... Traits> inline constexpr TraitKey join() {
  return detail::Join<Descriptor, Traits...>::value();
}

/// Joins local (equivalent to |) local keys for specified traits except a trait
/// 'Trait'.
template<class Trait, class Descriptor, class... Traits>
inline constexpr TraitKey joinWithout() {
  return detail::Join<Descriptor, Traits...>::template valueWithout<Trait>();
}

/// Calculates number of bits required to represent group of traits.
template<class... Traits> inline constexpr std::size_t bitSize() {
  return RemoveDuplicate<typename TraitList<Traits...>::Type>::Type::size_of();
}

/// Calculates index of a specified trait in a group of traits.
template<class Trait, class... Traits> inline constexpr std::size_t indexOf() {
  return RemoveDuplicate<
    typename TraitList<Traits...>::Type>::Type::template index_of<Trait>();
}

/// \brief Checks if a trait is contained in a group of traits.
///
/// If a trait is contained in a group provides the member constant `value`
/// equal to true, otherwise `value` is false.
template<class Trait, class... Traits> struct is_contained;

template<class Trait, class First, class... Traits>
struct is_contained<Trait, First, Traits...> :
  public std::conditional<is_contained<Trait, First>::value,
  std::true_type, is_contained<Trait, Traits...>>::type{};

/// Checks if a trait is contained in a group of traits.
template<class Trait, class First>
struct is_contained<Trait, First> : public std::is_same<Trait, First> {};

/// Checks if a trait is contained in a group of traits.
template<class Trait, class... Traits>
struct is_contained<Trait, TraitUnion<Traits...>> :
  public is_contained<Trait, Traits...> {};

/// Checks if a trait is contained in a group of traits.
template<class Trait, class... Traits>
struct is_contained<Trait, TraitAlternative<Traits...>> :
  public is_contained<Trait, Traits...> {};

/// Finds group of traits which contains a specified trait.
template<class Trait, class... Gropus> struct find_group;

/// Finds group of traits which contains a specified trait.
template<class Trait, class First, class... Groups>
struct find_group<Trait, First, Groups...> {
  typedef typename std::conditional<is_contained<Trait, First>::value,
    First, typename find_group<Trait, Groups...>::type>::type type;
};

/// Finds group of traits which contains a specified trait.
template<class Trait> struct find_group<Trait> {
  typedef void type;
};

/// Finds group of traits which contains a specified trait.
template<class Trait, class... Groups>
using find_group_t = typename find_group<Trait, Groups...>::type;

namespace detail {
/// \brief Returns a number of bits from the start of key to the end of a group
/// which contains a specified trait.
///
/// To encode multiple traits from different group a bit key is used. There are
/// continuous sequence of bits for each group in this key.
template<class Trait, class... Groups> struct BitsToTrait;

/// Returns a number of bits from the start of key to the end of a group
/// which contains a specified trait.
template<class Trait>
struct BitsToTrait<Trait> {
  static constexpr std::size_t value() { return 0; }
};

/// Returns a number of bits from the start of key to the end of a group
/// which contains a specified trait.
template<class Trait, class First, class... Groups>
struct BitsToTrait<Trait, First, Groups...> {
  static constexpr std::size_t value() {
    return (is_contained<Trait, First>::value ? 0 :
      BitsToTrait<Trait, Groups...>::value()) + bitSize<First>();
  }
};
}

/// \brief Finds lists of unions of traits which contain a specified trait.
///
/// This provides the type `type` equal to true, bcl::TypeList of TraitUnion and
/// single traits.
template<class Trait, class First, class... Groups> struct find_union {
  typedef typename bcl::MergeTypeLists<
    typename find_union<Trait, First>::type,
    typename find_union<Trait, Groups...>::type>::Type type;
};

/// Finds lists of unions of traits which contain a specified trait.
template<class Trait, class... Traits>
struct find_union<Trait, TraitAlternative<Traits...>> {
  typedef typename find_union<Trait, Traits...>::type type;
};

/// Finds lists of unions of traits which contain a specified trait.
template<class Trait, class... Traits>
struct find_union<Trait, TraitUnion<Traits...>> {
  typedef typename std::conditional<is_contained<Trait, Traits...>::value,
    bcl::TypeList<TraitUnion<Traits...>>, bcl::TypeList<>>::type type;
};

/// Finds lists of unions of traits which contain a specified trait.
template<class Trait, class First>
struct find_union<Trait, First> {
  typedef typename std::conditional<is_contained<Trait, First>::value,
    bcl::TypeList<TraitUnion<Trait>>, bcl::TypeList<>>::type type;
};

/// \brief Finds lists of unions of traits which contain a specified trait.
///
/// This provides the type `type` equal to true, bcl::TypeList of TraitUnion and
/// single traits.
template<class Trait, class... Groups>
using find_union_t = typename find_union<Trait, Groups...>::type;

/// \brief Returns a number of bits from the end of a group which contains a
/// specified trait to the end of a key.
///
/// To encode multiple traits from different group a bit key is used. There are
/// continuous sequence of bits for each group in this key. This function
/// returns a number of bits from the end of a sequence which contains a
/// specified trait to the end of the key.
template<class Trait, class... Groups>
inline constexpr std::size_t bitsToEnd() {
  static_assert(trait::is_contained<Trait, Groups...>::value,
    "Requested trait is not specified in either group!");
  return sizeof(TraitKey) * CHAR_BIT -
    detail::BitsToTrait<Trait, Groups...>::value();
}

/// Checks whether two traits (LHS and RHS) can be set simultaneously.
template<class LHS, class RHS, class... Groups>
struct is_conflict : public
  std::conditional<
    !std::is_same<
      find_group_t<LHS, Groups...>, find_group_t<RHS, Groups...>>::value,
    std::false_type,
    typename std::conditional<
      std::is_same<
        typename bcl::IntersectTypeLists<
          find_union_t<LHS, Groups...>, find_union_t<RHS, Groups...>>::Type,
        bcl::TypeList<>>::value,
      std::true_type,
      std::false_type>::type>::type {};

namespace detail {
/// This performs execution of a specified function for traits which is set
/// in a descriptor.
template<class Function, class Descriptor, class... Traits> struct ForEachIfSet;

/// This performs execution of a specified function for traits which is set
/// in a descriptor.
template<class Function, class Descriptor, class First, class... Traits>
struct ForEachIfSet<Function, Descriptor, First, Traits...> {
  /// \brief Executes the function for each trait which is set.
  static void exec(const Descriptor &TD, Function F) {
    ForEachIfSet<Function, Descriptor, First>::exec(TD, F);
    ForEachIfSet<Function, Descriptor, Traits...>::exec(TD, F);
  }
};

/// This performs execution of a specified function for traits which is set
/// in a descriptor.
template<class Function, class Descriptor, class... Traits>
struct ForEachIfSet<Function, Descriptor, TypeList<Traits...>> :
  public ForEachIfSet<Function, Descriptor, Traits...> {};

/// This performs execution of a specified function for traits which is set
/// in a descriptor.
template<class Function, class Descriptor, class Trait>
struct ForEachIfSet<Function, Descriptor, Trait> {
  /// Execute a specified function if a trait is set.
  static void exec(const Descriptor &TD, Function F) {
    if (!TD.template is<Trait>())
      return;
#ifdef __GNUC__
    F.template operator()<Trait>();
#else
    F.operator()<Trait> ();
#endif
  }
};

/// This performs execution of a specified function for traits which is set
/// in a descriptor.
template<class Function, class Descriptor>
struct ForEachIfSet<Function, Descriptor> {
  static void exec(const Descriptor &, Function) {}
};
}
}

namespace detail {
/// Recognizes traits in bcl::TypeList TList which are conflicted with Trait.
template<class Trait, class TList, class... Groups>
struct TraitConflictImp {
  typedef typename std::conditional<
    trait::is_conflict<Trait, typename TList::Type, Groups...>::value,
      typename MergeTypeLists<
        TypeList<typename TList::Type>,
        typename TraitConflictImp<
          Trait, typename TList::Next, Groups...>::Type>::Type,
      typename TraitConflictImp<
        Trait, typename TList::Next, Groups...>::Type>::type Type;
};

/// Recognizes traits in bcl::TypeList TList which are conflicted with Trait.
template<class Trait, class... Groups>
struct TraitConflictImp<Trait, TypeList<>, Groups...> {
  typedef TypeList<> Type;
};
}

/// Recognizes traits which are conflicted with a specified trait Trait.
template<class Trait, class... Groups>
struct TraitConflict {
  typedef typename detail::TraitConflictImp<
    Trait,
    typename RemoveDuplicate<typename TraitList<Groups...>::Type>::Type,
    Groups...>::Type Type;
};

/// \brief Represents a set of flags which specify whether some trait exist.
///
/// \tparam Groups A group of traits that can be specified. Each element of a
/// group is a single trait or list of alternative traits (TraitAlternative).
/// Each element of a list of alternative traits is a single trait or a list of
/// compliant traits (TraitUnion).
/// \pre Each single trait must be included in one group only.
///
/// Let us consider an example:
/// \code
///   sturct Alien {};
///   struct Secret {};
///   struct Man {};
///   struct Woman {};
///
///   TraitDescriptor<
///     Alien,
///     TraitAlternative<
///       TraitUnion<Secret, Man>,
///       TraitUnion<Secret, Woman>>> TD;
/// \endcode
/// There are two groups of traits:
/// - Alien is a single trait. It can be set or unset, by default it is unset.
/// - TraitAlternative<...> is a list of alternative traits. If someone want
/// to hide gender identity the Secret trait will be used simultaneously with
/// Man or Woman traits. But only one of this two traits can be used at the same
/// time. By default all traits is unset, this means that gender identity in
/// unknown and is not hidden.
template<class... Groups>
class TraitDescriptor {
public:
  /// \brief Returns a unique key for a trait in a trait list.
  template<class Trait> static constexpr TraitKey getKey() {
    static_assert(trait::is_contained<Trait, Groups...>::value,
      "Requested trait is not specified in either group!");
    return bcl::unitMask<TraitKey>(1) << (trait::bitsToEnd<Trait, Groups...>() +
      trait::bitSize<trait::find_group_t<Trait, Groups...>>() -
      trait::indexOf<Trait, trait::find_group_t<Trait, Groups...>>() - 1);
  }

  /// \brief Returns a mask for a group of traits which contains a specified
  /// trait.
  ///
  /// For a trait Man a mask is
  /// `011100000000000000000000000000000000000000000000000000000000000000`.
  template<class Trait> static constexpr TraitKey getMask() {
    static_assert(trait::is_contained<Trait, Groups...>::value,
      "Requested trait is not specified in either group!");
    return
      bcl::unitMask<TraitKey>(trait::bitsToEnd<Trait, Groups...>() +
        trait::bitSize<trait::find_group_t<Trait, Groups...>>()) &
      ~bcl::unitMask<TraitKey>(trait::bitsToEnd<Trait, Groups...>());
  }

  /// Joins (equivalent to |) all specified keys.
  template<class Trait> static constexpr TraitKey joinKey() {
    return getKey<Trait>();
  }

  /// Joins (equivalent to |) all specified keys.
  template<class First, class Second, class... Traits>
  static constexpr TraitKey joinKey() {
    return joinKey<First>() | joinKey<Second, Traits...>();
  }

  /// Joins (equivalent to |) masks for groups which contain specified traits.
  template<class Trait> static constexpr TraitKey joinMask() {
    return getMask<Trait>();
  }

  /// Joins (equivalent to |) masks for groups which contain specified traits.
  template<class First, class Second, class... Traits>
  static constexpr TraitKey joinMask() {
    return joinMask<First>() | joinMask<Second, Traits...>();
  }

  /// Joins (equivalent to |) traits which are conflicted with specified ones.
  template<class First> static constexpr TraitKey joinConflict() {
    return trait::join<TraitDescriptor,
      typename TraitConflict<First, Groups...>::Type>();
  }

  /// Joins (equivalent to |) traits which are conflicted with specified ones.
  template<class First, class Second, class... Traits>
  static constexpr TraitKey joinConflict() {
    return joinConflict<First>() | joinConflict<Second, Traits...>();
  }

  /// Set traits and unset all conflicted traits.
  template<class... Traits> void set() {
    auto constexpr ConflictMasks = ~joinConflict<Traits...>();;
    auto constexpr Keys = joinKey<Traits...>() & ConflictMasks;
    mTD = mTD & ConflictMasks | Keys;
  }

  /// Unset all specified traits.
  template<class... Traits> void unset() {
    auto constexpr NotKeys = ~joinKey<Traits...>();
    mTD = mTD & NotKeys;
  }

  /// Unset groups which contains specified traits.
  template<class... Traits> void unset_group() {
    auto constexpr NotMasks = ~joinMask<Traits...>();
    mTD = mTD & NotMasks;
  }

  /// Unset all traits.
  void unset_all() { mTD = 0; }

  /// Checks whether all specified traits are set.
  template<class... Traits> bool is() const {
    auto constexpr Keys = joinKey<Traits...>();
    return mTD & Keys;
  }

  /// \brief Executes function for each trait which is set and conflicts with
  /// a specified trait.
  ///
  /// \pre The `template<class Trait> void operator()()` method
  /// must be defined in the \c Function class.
  template<class Trait, class Function>
  void for_each_conflict(Function &&F) const {
    trait::detail::ForEachIfSet<Function, TraitDescriptor,
      typename TraitConflict<Trait, Groups...>::Type>::exec(*this, F);
  }

  /// \brief Executes function for each trait which is set and belongs to a
  /// group that contains a specified trait.
  ///
  /// \pre The `template<class Trait> void operator()()` method
  /// must be defined in the \c Function class.
  template<class Trait, class Function>
  void for_each_in_group(Function &&F) const {
    trait::detail::ForEachIfSet<Function, TraitDescriptor,
      typename RemoveDuplicate<
        typename TraitList<trait::find_group_t<Trait, Groups...>>::Type>::Type>
    ::exec(*this, F);
  }

  /// \brief Executes function for each trait which is set.
  ///
  /// \pre The `template<class Trait> void operator()()` method
  /// must be defined in the \c Function class.
  template<class Function> void for_each(Function &&F) const {
    trait::detail::ForEachIfSet<Function, TraitDescriptor,
      typename RemoveDuplicate<typename TraitList<Groups...>::Type>::Type>
    ::exec(*this, F);
  }

  /// \brief Executes function for each available trait.
  ///
  /// \pre The `template<class Trait> void operator()()` method
  /// must be defined in the \c Function class.
  template<class Function> static void for_each_available(Function &&F) {
    RemoveDuplicate<typename TraitList<Groups...>::Type>::Type
      ::for_each_type(std::forward<Function>(F));
  }

  /// Prints bit representation of descriptor.
  template<class OutputStream>
  void print(OutputStream &OS) const {
    OS << std::bitset<sizeof(TraitKey) * CHAR_BIT>(mTD).to_string();
  }

  /// Prints bit representation of unique keys.
  template<class... Traits, class OutputStream>
  static void printKey(OutputStream &OS) {
    auto constexpr Keys = joinKey<Traits...>();
    OS << std::bitset<sizeof(TraitKey) * CHAR_BIT>(Keys).to_string();
  }

  /// Prints bit representation of masks.
  template<class... Traits, class OutputStream>
  static void printMask(OutputStream &OS) {
    auto constexpr Masks = joinMask<Traits...>();
    OS << std::bitset<sizeof(TraitKey) * CHAR_BIT>(Masks).to_string();
  }

  TraitKey mTD = 0;
};

/// \brief Constructs list of all traits belongs to a specified descriptor.
///
/// Note that this does not remove duplicate traits, to remove them use
/// bcl::RemoveDuplicate<typename TraitList<...>::Type>.
template<class... Groups> struct TraitList<TraitDescriptor<Groups...>> {
  typedef typename TraitList<Groups...>::Type Type;
};

namespace trait {
/// \brief Checks whether two traits (LHS and RHS) can be set simultaneously.
///
/// This is a specialization for a TraitDescriptor<...> class.
template<class LHS, class RHS, class... Groups>
struct is_conflict<LHS, RHS, TraitDescriptor<Groups...>> :
  public is_conflict<LHS, RHS, Groups...>::type {};
}

/// \brief This represents set of traits.
///
/// In addition to a trait descriptor which propose information whether a trait
/// is set or not, some description of traits which is set are available.
/// \tparam TraitDescriptor Descriptor of traits.
/// \tparam TraitMap A map from TraitKey to void * which is used to store
/// description of each trait.
template<class TraitDescriptor, class TraitMap>
class TraitSet {
  template<class NewTrait>
  struct ConflictsResolver {
    ConflictsResolver(TraitMap *Values) : mValues(Values) {
      assert(mValues && "Map of trait descriptions must not be null!");
    }

    template<class Trait>
    void operator()() {
      auto constexpr Key = TraitDescriptor::template getKey<Trait>();
      auto I = mValues->find(Key);
      if (I != mValues->end()) {
        if (I->second)
          delete reinterpret_cast<Trait*>(I->second);
        mValues->erase(I);
      }
    }

  private:
    TraitMap *mValues;
  };

public:
  /// Creates set of traits.
  TraitSet(const TraitDescriptor &TD) : mTD(TD) {}

  /// Creates set of traits.
  TraitSet(TraitDescriptor &&TD) : mTD(std::move(TD)) {}

  /// Checks whether a trait is set.
  template<class... Traits> bool is() const { return mTD.is<Traits...>(); }

  /// \brief Executes function for each trait which is set and belongs to a
  /// group that contains a specified trait.
  ///
  /// \pre The `template<class Trait> void operator()(TraitDescriptor &)` method
  /// must be defined in the \c Function class.
  template<class Trait, class Function>
  void for_each_in_group(Function &&F) const {
    mTD.for_each_in_group<Trait>(std::forward<Function>(F));
  }

  /// \brief Executes function for each trait which is set.
  ///
  /// \pre The `template<class Trait> void operator()()` method
  /// must be defined in the \c Function class.
  template<class Function> void for_each(Function &&F) const {
    mTD.for_each(std::forward<Function>(F));
  }

  /// \brief Executes function for each trait which is set and conflicts with
  /// a specified trait.
  ///
  /// \pre The `template<class Trait> void operator()()` method
  /// must be defined in the \c Function class.
  template<class Trait, class Function>
  void for_each_conflict(Function &&F) const {
    mTD.for_each_conflict<Trait>(std::forward<Function>(F));
  }

  /// \brief Executes function for each available trait.
  ///
  /// \pre The `template<class Trait> void operator()()` method
  /// must be defined in the \c Function class.
  template<class Function> static void for_each_available(Function &&F) {
    TraitDescriptor::for_each_available(std::forward<Function>(F));
  }

  /// \brief Add description of a specified trait.
  ///
  /// If the trait has not be set in descriptor it will be set and all
  /// conflicted traits will be unset.
  /// \note This class manages memory allocation to store description of traits.
  template<class Trait> void set(Trait *T) {
    auto constexpr Key = mTD.template getKey<Trait>();
    auto I = mValues.find(Key);
    if (I != mValues.end()) {
      if (I->second)
        delete reinterpret_cast<Trait*>(I->second);
      I->second = reinterpret_cast<void *>(T);
      return;
    }
    if (!mValues.empty())
      mTD.template for_each_conflict<Trait>(ConflictsResolver<Trait>(&mValues));
    mTD.set<Trait>();
    mValues.insert(
      std::make_pair(mTD.template getKey<Trait>(), reinterpret_cast<void *>(T)));
  }

  /// \brief Returns description of a specified trait or nullptr.
  ///
  /// If nullptr is returned it does not mean that a specified trait is not set,
  /// in a descriptor it means that appropriate description has not been
  /// initialized.
  template<class Trait> Trait * get() {
    auto constexpr Key = mTD.template getKey<Trait>();
    auto I = mValues.find(Key);
    return I == mValues.end() ? nullptr : reinterpret_cast<Trait *>(I->second);
  }

  /// \brief Removes description of a specified trait from this set and
  /// returns it.
  ///
  /// If nullptr is returned it does not mean that a specified trait is not set,
  /// in a descriptor it means that appropriate description has not been
  /// initialized.
  /// If the trait has been set it will not be unset in descriptor but
  /// description will be removed from this set.
  template<class Trait> Trait * release() {
    auto constexpr Key = mTD.template getKey<Trait>();
    auto I = mValues.find(Key);
    Trait *Result =
      I == mValues.end() ? nullptr : reinterpret_cast<Trait *>(I->second);
    mValues.erase(I);
    return Result;
  }

  /// Prints bit representation of descriptor.
  template<class OutputStream>
  void print(OutputStream &OS) const { mTD.print(OS); }

private:
  TraitMap mValues;
  TraitDescriptor mTD;
};

/// \brief This is a static map where each trait in descriptor is treated as
/// a key with value type equal to Ty.
///
/// This is similar to bcl::StaticMap.
/// \tparam Ty Type of values stored in the map.
/// \tparam TraitDescriptor Set of available traits (see TraitDescriptor<...>).
template<class Ty, class TraitDescriptor> class StaticTraitMap {
  /// Wrapper which constructs a key in map for a specified trait.
  template<class Trait> struct StaticMapKey {
    typedef Ty ValueType;
    typedef Trait TraitType;
  };

  /// Helper class to constructs keys.
  template<class Trait> struct KeyConstructor {
    typedef StaticMapKey<Trait> CellKey;
  };

  /// This is a bcl::StaticMap<...> which stores value of the Ty type.
  typedef typename bcl::detail::StaticMapConstructorImp<KeyConstructor,
    typename RemoveDuplicate<
      typename TraitList<TraitDescriptor>::Type>::Type>::Type MapType;

  /// Helper class to implement for_each() methods.
  template<class T> class FunctorWrapper {
  public:
    FunctorWrapper(T &F) : mFunction(F) {}

    /// TODO (kaniandr@gmail.com): Remove __GNUC__.
    template<class CellTy> void operator()(CellTy *C) {
      typedef typename CellTy::CellKey CellKey;
      typedef typename CellTy::ValueType ValueType;
      typedef typename CellKey::TraitType TraitType;
#ifdef __GNUC__
      mFunction.template operator()<TraitType>(C->value<CellKey>());
#else
      mFunction.operator()<TraitType>(C->value<CellKey>());
#endif
    }

  private:
    T &mFunction;
  };

  /// Helper class to implement for_each_key() methods.
  template<class T> class KeyFunctorWrapper {
  public:
    KeyFunctorWrapper(T &F) : mFunction(F) {}

    /// TODO (kaniandr@gmail.com): Remove __GNUC__.
    template<class CellTy> void operator()() {
      typedef typename CellTy::CellKey CellKey;
      typedef typename CellKey::TraitType TraitType;
#ifdef __GNUC__
      mFunction.template operator()<TraitType>();
#else
      mFunction.operator()<TraitType>();
#endif
    }
  private:
    T &mFunction;
  };

public:
  /// \brief Applies a specified function to each trait in the map.
  ///
  /// \pre The `template<class Trait> void operator()()` method must be defined
  /// in the \c Function class.
  /// TODO (kaniandr@gmail.com): Override and make it possible to specify a list
  /// of functions.
  template<class Function> static void for_each_key(Function &&F) {
    KeyFunctorWrapper<Function> Wrapper(F);
    MapType::for_each_key(Wrapper);
  }

  /// Returns a value of type Ty related to a specified trait.
  template<class Trait> Ty & value() {
    return mMap.value<StaticMapKey<Trait> >();
  }

  /// Returns a value of type Ty related to a specified trait.
  template<class Trait> const Ty & value() const {
    return mMap.value<StaticMapKey<Trait> >();
  }

  /// Returns a value of type Ty related to a specified trait.
  template<class Trait> Ty & operator[](Trait) {
    return mMap.value<StaticMapKey<Trait>>();
  }

  /// Returns a value of type Ty related to a specified trait.
  template<class Trait> const Ty & operator[](Trait) const {
    return mMap.value<StaticMapKey<Trait>>();
  }

  /// \brief Applies a specified function to each trait in the map.
  ///
  /// \pre The `template<class Trait> void operator()(Ty &V)` method must be
  /// defined in the \c Function class.
  /// TODO (kaniandr@gmail.com): Override and make it possible to specify a list
  /// of functions.
  template<class Function> void for_each(Function &&F) {
    FunctorWrapper<Function> Wrapper(F);
    mMap.for_each(Wrapper);
  }

  /// Applies a specified function to each trait in the map.
  template<class Function> void for_each(Function &&F) const {
    FunctorWrapper<Function> Wrapper(F);
    mMap.for_each(Wrapper);
  }

private:
  MapType mMap;
};

/// \brief This functor distributes trait set to cells in a static trait map.
///
/// This functor stores a set of traits (TraitSet) in cells of a static map
/// (TraitMap). This map should contain a unique cell for each trait in a trait
/// descriptor (for example, TraitMap is bcl::StaticTraitMap<...>). If a trait
/// is set in the set of traits it will be stored in appropriate cell. The
/// template class Inserter must provide a static method
/// `insert(Coll &, Element &)` where Element is equal to TraitSet * and Coll is
/// a type of any cell in TraitMap.
///
/// Usage: `TraitSet Set; TraitMap Map; Set.for_each(Constructor(Set, Map))`.
template<class TraitSet, class TraitMap,
  template<class Coll, class Element> class Inserter = PushBackInserter>
class TraitMapConstructor {
public:
  /// Creates the functor.
  TraitMapConstructor(TraitSet &TS, TraitMap &Map) : mTS(&TS), mMap(&Map) {}

  /// Stores representation of a trait in a static map.
  template<class Trait> void operator()() {
    Inserter<
      typename std::remove_reference<decltype(mMap->template value<Trait>())>::type,
      TraitSet *>::insert(mMap->template value<Trait>(), mTS);
  }

  /// Returns a static trait map.
  TraitMap & getTraitMap() { return *mMap; }

  /// Returns a trait set.
  TraitSet & getTraitSet() { return *mTS; }

private:
  TraitMap *mMap;
  TraitSet *mTS;
};
}
#endif//TRAIT_H

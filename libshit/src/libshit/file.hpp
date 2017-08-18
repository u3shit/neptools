#ifndef UUID_4C4561D8_78E4_438B_9804_61F42DB159F7
#define UUID_4C4561D8_78E4_438B_9804_61F42DB159F7
#pragma once

#include "meta_utils.hpp"

#include <brigand/algorithms/fold.hpp>
#include <brigand/algorithms/split.hpp>
#include <brigand/functions/eval_if.hpp>
#include <brigand/functions/if.hpp>
#include <brigand/functions/misc/always.hpp>
#include <brigand/sequences/back.hpp>
#include <brigand/sequences/list.hpp>

namespace Libshit
{

  template <char... Args> struct StringContainer
  {
    template <char C> using PushBack = StringContainer<Args..., C>;

    static constexpr const char str[sizeof...(Args)+1] = { Args... };
    static constexpr const wchar_t wstr[sizeof...(Args)+1] = { Args... };
  };

  template <char... Args>
  constexpr const char StringContainer<Args...>::str[];

  template <char... Args>
  constexpr const wchar_t StringContainer<Args...>::wstr[];


  namespace FileTools
  {

    template <char Val> using C = brigand::integral_constant<char, Val>;
    template <char... Vals> using CL = brigand::integral_list<char, Vals...>;

    template <typename Elem, typename State> struct Fold
    { using type = brigand::push_back<State, Elem>; };
    // ignore .
    template <typename State> struct Fold<CL<'.'>, State>
    { using type = State; };
    // .. eats a directory
    template <typename State> struct Fold<CL<'.','.'>, State>
    { using type = brigand::pop_back<State>; };
    // except if state already empty
    template <> struct Fold<CL<'.','.'>, CL<>> { using type = CL<>; };

    // ignore everything before src/ext
    template <typename State> struct Fold<CL<'s','r','c'>, State>
    { using type = CL<>; };
    template <typename State> struct Fold<CL<'e','x','t'>, State>
    { using type = CL<>; };


    template <typename List, typename Sep, typename Bld>
    struct LJoin;

    template <typename List, typename Sep, typename Bld = brigand::list<>>
    using Join = typename LJoin<List, Sep, Bld>::type;


    template <typename Sep, typename Bld>
    struct LJoin<brigand::list<>, Sep, Bld>
    { using type = Bld; };

    template <typename LHead, typename... LTail, typename Sep>
    struct LJoin<brigand::list<LHead, LTail...>, Sep, brigand::list<>>
    { using type = Join<brigand::list<LTail...>, Sep, LHead>; };

    template <typename LHead, typename... LTail, typename Sep, typename Bld>
    struct LJoin<brigand::list<LHead, LTail...>, Sep, Bld>
    { using type = Join<brigand::list<LTail...>, Sep,
                        brigand::append<Bld, Sep, LHead>>; };


    template <typename X> struct LWrap;
    template <char... Chars> struct LWrap<CL<Chars...>>
    { using type = StringContainer<Chars...>; };

    template <typename X> using Wrap = typename LWrap<X>::type;

    template <char... Args>
    using FileName =
      Wrap<Join<brigand::fold<
                  brigand::split<CL<Args...>, C<'/'>>, CL<>,
                  Fold<brigand::_element, brigand::_state>>, CL<'/'>>>;

  }
}

#define LIBSHIT_FILE \
  LIBSHIT_LITERAL_CHARPACK(::Libshit::FileTools::FileName, __FILE__).str
#define LIBSHIT_WFILE \
  LIBSHIT_LITERAL_CHARPACK(::Libshit::FileTools::FileName, __FILE__).wstr

// boost doesn't check __clang__, and falls back to some simpler implementation
#if defined(__GNUC__) || defined(__clang__)
#  define LIBSHIT_FUNCTION __PRETTY_FUNCTION__
#else
#  include <boost/current_function.hpp>
#  define LIBSHIT_FUNCTION BOOST_CURRENT_FUNCTION
#endif

#endif

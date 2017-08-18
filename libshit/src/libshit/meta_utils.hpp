#ifndef UUID_EA2FC01F_70AB_4D47_AB3B_D3EF94DA7AB5
#define UUID_EA2FC01F_70AB_4D47_AB3B_D3EF94DA7AB5
#pragma once

#include <brigand/sequences/list.hpp>

#include <type_traits>
#include <utility>

namespace Libshit
{

  template <typename T>
  constexpr std::size_t EmptySizeof = std::is_empty<T>::value ? 0 : sizeof(T);

  // string->template char...
  template <typename X, template <char...> typename Wrap, typename Seq>
  struct ToCharPack;
  template <typename X, template <char...> typename Wrap, size_t... Idx>
  struct ToCharPack<X, Wrap, std::index_sequence<Idx...>>
  {
    using Type = Wrap<X::Get(Idx)...>;
  };

  template <typename X, template <char...> typename Wrap, typename Seq>
  using ToCharPackV = typename ToCharPack<X, Wrap, Seq>::Type;

#define LIBSHIT_LITERAL_CHARPACK(type, str)                                \
  ([]{                                                                     \
    struct X { static constexpr char Get(size_t i) { return (str)[i]; } }; \
    return ::Libshit::ToCharPackV<                                         \
      X, type, std::make_index_sequence<sizeof(str)-1>>{};                 \
  }())


  // function info
  template <typename T> struct FunctionTraits;
  template <typename Ret, typename... Args> struct FunctionTraits<Ret(Args...)>
  {
    using Return = Ret;
    using Arguments = brigand::list<Args...>;
  };

  template <typename Ret, typename... Args>
  struct FunctionTraits<Ret(*)(Args...)>
    : FunctionTraits<Ret(Args...)> {};
  template <typename Ret, typename C, typename... Args>
  struct FunctionTraits<Ret(C::*)(Args...)>
    : FunctionTraits<Ret(C&, Args...)> {};
  template <typename Ret, typename C, typename... Args>
  struct FunctionTraits<Ret(C::*)(Args...) const>
    : FunctionTraits<Ret(const C&, Args...)> {};

  template <typename Ret, typename... Args>
  struct FunctionTraits<Ret(*)(Args...) noexcept>
    : FunctionTraits<Ret(Args...)> {};
  template <typename Ret, typename C, typename... Args>
  struct FunctionTraits<Ret(C::*)(Args...) noexcept>
    : FunctionTraits<Ret(C&, Args...)> {};
  template <typename Ret, typename C, typename... Args>
  struct FunctionTraits<Ret(C::*)(Args...) const noexcept>
    : FunctionTraits<Ret(const C&, Args...)> {};


  template <typename T>
  using FunctionReturn = typename FunctionTraits<T>::Return;
  template <typename T>
  using FunctionArguments = typename FunctionTraits<T>::Arguments;

}

#endif

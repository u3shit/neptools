#ifndef UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#define UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#pragma once

#ifndef LIBSHIT_WITHOUT_LUA

#include "function_call_types.hpp"
#include "type_traits.hpp"
#include "../meta_utils.hpp"

#include <brigand/sequences/list.hpp>

namespace Libshit::Lua
{

  template <typename... Args> struct TupleLike<std::tuple<Args...>>
  {
    using Tuple = std::tuple<Args...>;
    template <size_t I> static decltype(auto) Get(const Tuple& t)
    { return std::get<I>(t); }
    static constexpr size_t SIZE = sizeof...(Args);
  };

  template <typename T>
  using EnableIfTupleLike = std::void_t<decltype(TupleLike<T>::SIZE)>;

  namespace Detail
  {

    inline constexpr size_t IDX_VARARG =
      size_t{1} << (std::numeric_limits<size_t>::digits-1);
    inline constexpr size_t IDX_MASK = IDX_VARARG - 1;

    template <typename T, typename Enable = void> struct GetArgImpl
    {
      template <size_t Idx> static constexpr size_t NEXT_IDX = Idx+1;

      template <bool Unsafe> static decltype(auto) Get(StateRef vm, int idx)
      { return vm.Check<T, Unsafe>(idx); }
      static bool Is(StateRef vm, int idx) { return vm.Is<T>(idx); }

      static void Print(bool comma, std::ostream& os)
      {
        if (comma) os << ", ";
        TypeTraits<T>::PrintName(os);
      }
    };

    template <> struct GetArgImpl<Skip>
    {
      template <size_t Idx> static constexpr size_t NEXT_IDX = Idx+1;
      template <bool>
      static constexpr Skip Get(StateRef, int) noexcept { return {}; }
      static constexpr bool Is(StateRef, int) noexcept { return true; }
      static void Print(bool, std::ostream&) {}
    };

    template <> struct GetArgImpl<VarArg>
    {
      template <size_t Idx> static constexpr size_t NEXT_IDX = IDX_VARARG | Idx;
      template <bool> static constexpr VarArg Get(StateRef, int) noexcept
      { return {}; }
      static constexpr bool Is(StateRef, int) noexcept { return true; }
      static void Print(bool comma, std::ostream& os)
      {
        os << (comma ? ", ..." : "...");
      }
    };

    template <> struct GetArgImpl<StateRef>
    {
      template <size_t Idx> static constexpr size_t NEXT_IDX = Idx;
      template <bool>
      static constexpr StateRef Get(StateRef vm, int) noexcept { return vm; }
      static constexpr bool Is(StateRef, int) noexcept { return true; }
      static void Print(bool, std::ostream&) {}
    };

    template <> struct GetArgImpl<Any>
    {
      template <size_t Idx> static constexpr size_t NEXT_IDX = Idx+1;
      template <bool>
      static constexpr Any Get(StateRef, int idx) noexcept { return {idx}; }
      static constexpr bool Is(StateRef, int) noexcept { return true; }
      static void Print(bool, std::ostream&) {}

      using type = brigand::list<>;
      template <typename Val>
      static constexpr const bool IS = true;
    };

    template <int LType> struct GetArgImpl<Raw<LType>>
    {
      template <size_t Idx> static constexpr size_t NEXT_IDX = Idx+1;
      template <bool Unsafe>
      static Raw<LType> Get(StateRef vm, int idx)
      {
        if (!Unsafe && !Is(vm, idx))
          vm.TypeError(true, lua_typename(vm, LType), idx);
        return {idx};
      }
      static bool Is(StateRef vm, int idx) noexcept
      { return lua_type(vm, idx) == LType; }
      static void Print(bool comma, std::ostream& os)
      {
        if (comma) os << ", ";
        os << lua_typename(nullptr, LType);
      }
    };

    template <typename Tuple, size_t I>
    using TupleElement = std::decay_t<decltype(
      TupleLike<Tuple>::template Get<I>(std::declval<Tuple>()))>;

    template <typename Tuple, typename Index> struct TupleGet;
    template <typename Tuple, size_t... Index>
    struct TupleGet<Tuple, std::index_sequence<Index...>>
    {
      template <size_t Idx> static constexpr size_t NEXT_IDX =
        Idx + sizeof...(Index);

      template <bool Unsafe>
      static Tuple Get(StateRef vm, int idx)
      { return {vm.Get<TupleElement<Tuple, Index>, Unsafe>(idx+Index)...}; }

      static bool Is(StateRef vm, int idx)
      { return (vm.Is<TupleElement<Tuple, Index>>(idx+Index) && ...); }

      static void Print(bool comma, std::ostream& os)
      {
        ((((comma || Index != 0) && os << ", "),
          TypeTraits<TupleElement<Tuple, Index>>::PrintName(os)), ...);
      }
    };

    template <typename T>
    struct GetArgImpl<T, EnableIfTupleLike<T>>
      : TupleGet<T, std::make_index_sequence<TupleLike<T>::SIZE>> {};

    template <typename T> using GetArg = GetArgImpl<std::decay_t<T>>;


    template <size_t I, typename Argument> struct Arg
    {
      static constexpr const size_t Idx = I & IDX_MASK;
      using ArgT = Argument;
    };


    template <size_t Len, typename Args> struct ArgSeq;

    template <size_t N, typename Seq, typename Args>
    struct GenArgSequence;

    template <size_t N, typename... Seq, typename Head, typename... Args>
    struct GenArgSequence<N, brigand::list<Seq...>, brigand::list<Head, Args...>>
    {
      using Type = typename GenArgSequence<
        GetArg<Head>::template NEXT_IDX<N>,
        brigand::list<Seq..., Arg<N, Head>>,
        brigand::list<Args...>>::Type;
    };
    template <size_t N, typename Seq>
    struct GenArgSequence<N, Seq, brigand::list<>>
    { using Type = ArgSeq<N-1, Seq>; };

    template <auto Fun>
    using ArgSequence = typename GenArgSequence<
      1, brigand::list<>, FunctionArguments<decltype(Fun)>>::Type;

    template <typename Args>
    using ArgSequenceFromArgs = typename GenArgSequence<
      1, brigand::list<>, Args>::Type;



    template <typename T, typename Enable = void> struct ResultPush
    {
      template <typename U>
      static int Push(StateRef vm, U&& t)
      {
        vm.Push<T>(std::forward<U>(t));
        return 1;
      }
    };

    template<> struct ResultPush<RetNum>
    { static int Push(StateRef, RetNum r) { return r.n; } };

    template <typename Tuple, typename Index> struct TuplePush;
    template <typename Tuple, size_t... I>
    struct TuplePush<Tuple, std::index_sequence<I...>>
    {
      static int Push(StateRef vm, const Tuple& ret)
      {
        (vm.Push(TupleLike<Tuple>::template Get<I>(ret)), ...);
        return sizeof...(I);
      }
    };

    template<typename T> struct ResultPush<T, EnableIfTupleLike<std::decay_t<T>>>
      : TuplePush<std::decay_t<T>,
                  std::make_index_sequence<TupleLike<std::decay_t<T>>::SIZE>> {};

    // workaround gcc can't mangle noexcept template arguments...
    template <typename... Args>
    struct NothrowInvokable : std::integral_constant<
      bool, noexcept(Invoke(std::declval<Args>()...))> {};

    template <typename... Args>
    BOOST_FORCEINLINE
    auto CatchInvoke(StateRef, Args&&... args) -> typename std::enable_if<
      NothrowInvokable<Args&&...>::value,
      decltype(Invoke(std::forward<Args>(args)...))>::type
    { return Invoke(std::forward<Args>(args)...); }

    inline void ToLuaException(StateRef vm)
    {
      auto s = ExceptionToString();
      lua_pushlstring(vm, s.data(), s.size());
      lua_error(vm);
    }

    template <typename... Args>
    auto CatchInvoke(StateRef vm, Args&&... args) -> typename std::enable_if<
      !NothrowInvokable<Args&&...>::value,
      decltype(Invoke(std::forward<Args>(args)...))>::type
    {
      try { return Invoke(std::forward<Args>(args)...); }
      catch (const std::exception& e)
      {
        ToLuaException(vm);
        LIBSHIT_UNREACHABLE("lua_error returned");
      }
    }

    template <auto Fun, bool Unsafe, typename Ret, typename Args>
    struct WrapFunGen;

    template <auto Fun, bool Unsafe, typename Ret, size_t N, typename... Args>
    struct WrapFunGen<Fun, Unsafe, Ret, ArgSeq<N, brigand::list<Args...>>>
    {
      static int Func(lua_State* l)
      {
        StateRef vm{l};
        return ResultPush<Ret>::Push(
          vm, CatchInvoke(
            vm, Fun, GetArg<typename Args::ArgT>::template Get<Unsafe>(
              vm, Args::Idx)...));
      }
    };

    template <auto Fun, bool Unsafe, size_t N, typename... Args>
    struct WrapFunGen<Fun, Unsafe, void, ArgSeq<N, brigand::list<Args...>>>
    {
      static int Func(lua_State* l)
      {
        StateRef vm{l};
        CatchInvoke(vm, Fun, GetArg<typename Args::ArgT>::template Get<Unsafe>(
                      vm, Args::Idx)...);
        return 0;
      }
    };

    template <auto Fun, bool Unsafe>
    struct WrapFunc : WrapFunGen<
      Fun, Unsafe, FunctionReturn<decltype(Fun)>, ArgSequence<Fun>> {};

    // allow plain old lua functions
    template <int (*Fun)(lua_State*), bool Unsafe>
    struct WrapFunc<Fun, Unsafe>
    { static constexpr const auto Func = Fun; };


    // overload
    template <auto... args> struct AutoList;

    template <typename Args> struct OverloadCheck;
    template <size_t N, typename... Args>
    struct OverloadCheck<ArgSeq<N, brigand::list<Args...>>>
    {
      static bool Is(StateRef vm)
      {
        auto top = lua_gettop(vm);
        if (N & IDX_VARARG && size_t(top) >= N & IDX_MASK) return false;
        if (!(N & IDX_VARARG) && size_t(top) != N)         return false;

        return (GetArg<typename Args::ArgT>::Is(vm, Args::Idx) && ...);
      }
    };

    template <typename Funs, typename Orig = Funs> struct OverloadWrap;
    template <auto Fun, auto... Rest, typename Orig>
    struct OverloadWrap<AutoList<Fun, Rest...>, Orig>
    {
      static int Func(lua_State* l)
      {
        StateRef vm{l};
        if (OverloadCheck<ArgSequence<Fun>>::Is(vm))
          return WrapFunc<Fun, true>::Func(vm);
        else
          return OverloadWrap<AutoList<Rest...>, Orig>::Func(vm);
      }
    };

    template <typename Args> struct PrintOverload;
    template <size_t N, typename... Args>
    struct PrintOverload<ArgSeq<N, brigand::list<Args...>>>
    {
      static void Print(std::ostream& os)
      {
        os << "\n(";
        (GetArg<typename Args::ArgT>::Print(Args::Idx != 1, os), ...);
        os << ')';
      }
    };

    inline void PrintOverloadCommon(Lua::StateRef vm, std::stringstream& ss)
    {
      ss << "Invalid arguments (";
      auto top = lua_gettop(vm);
      for (int i = 0; i < top; ++i)
      {
        if (i != 0) ss << ", ";
        ss << vm.TypeName(i+1);
      }

      ss << ") to overloaded function. Overloads:";
    }

    template <auto... Funs>
    struct OverloadWrap<AutoList<>, AutoList<Funs...>>
    {
#if defined(__GNUC__) || defined(__clang__)
      __attribute__((minsize))
#endif
      static int Func(lua_State* l)
      {
        Lua::StateRef vm{l};
        std::stringstream ss;

        PrintOverloadCommon(vm, ss);
        (PrintOverload<ArgSequence<Funs>>::Print(ss), ...);
        return luaL_error(vm, ss.str().c_str());
      }
    };

    template <auto... Funs>
    using Overload = OverloadWrap<AutoList<Funs...>, AutoList<Funs...>>;

  }

  template <auto... Funs>
  inline void StateRef::PushFunction()
  {
    if constexpr (sizeof...(Funs) == 1)
                   lua_pushcfunction(vm, (Detail::WrapFunc<Funs..., false>::Func));
    else
      lua_pushcfunction(vm, Detail::Overload<Funs...>::Func);
  }

}

#endif
#endif

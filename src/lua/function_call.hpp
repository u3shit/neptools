#ifndef UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#define UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#pragma once

#ifndef NEPTOOLS_WITHOUT_LUA

#include "function_call_types.hpp"
#include "type_traits.hpp"
#include "../meta_utils.hpp"

#include <brigand/sequences/at.hpp>

#ifdef NEPTOOLS_LUA_OVERLOAD_CHECK
#include <brigand/algorithms/index_of.hpp>
#include <brigand/algorithms/flatten.hpp>
#include <brigand/algorithms/fold.hpp>
#include <brigand/algorithms/transform.hpp>
#include <brigand/functions/arithmetic/max.hpp>
#include <brigand/sequences/set.hpp>
#endif

namespace Neptools::Lua
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

template <typename T, typename Enable = void> struct GetArg;
template <typename T, typename> struct GetArg
{
    using RawType = typename std::decay<T>::type;
    using type = brigand::list<RawType>;

    template <size_t Idx> static constexpr size_t NEXT_IDX = Idx+1;

    template <bool Unsafe> static decltype(auto) Get(StateRef vm, int idx)
    { return vm.Check<RawType, Unsafe>(idx); }
    static bool Is(StateRef vm, int idx) { return vm.Is<RawType>(idx); }

    template <typename Val>
    static constexpr const bool IS = COMPATIBLE_WITH<RawType, Val>;
};

template <> struct GetArg<Skip>
{
    template <size_t Idx> static constexpr size_t NEXT_IDX = Idx+1;
    template <bool>
    static constexpr Skip Get(StateRef, int) noexcept { return {}; }
    static constexpr bool Is(StateRef, int) noexcept { return true; }

    using type = brigand::list<>;
    template <typename Val>
    static constexpr const bool IS = true;
};

template <> struct GetArg<StateRef>
{
    template <size_t Idx> static constexpr size_t NEXT_IDX = Idx;
    template <bool>
    static constexpr StateRef Get(StateRef vm, int) noexcept { return vm; }
    static constexpr bool Is(StateRef, int) noexcept { return true; }

    using type = brigand::list<>;
    template <typename Val>
    static constexpr const bool IS = true;
};

template <int LType> struct GetArg<Raw<LType>>
{
    template <size_t Idx> static constexpr size_t NEXT_IDX = Idx+1;
    template <bool Unsafe>
    static Raw<LType> Get(StateRef vm, int idx)
    {
        if (!Unsafe && !Is(vm, idx))
            vm.TypeError(true, lua_typename(vm, LType), idx);
        return {};
    }
    static bool Is(StateRef vm, int idx) noexcept
    { return lua_type(vm, idx) == LType; }

    using type = brigand::list<Raw<LType>>;
    template <typename Val>
    static constexpr const bool IS = std::is_same_v<Raw<LType>, Val>;
};

template <typename Tuple, size_t I>
using TupleElement = std::decay_t<decltype(
    TupleLike<Tuple>::template Get<I>(std::declval<Tuple>()))>;

template <typename Tuple, typename Index> struct TupleGet;
template <typename Tuple, size_t... Index>
struct TupleGet<Tuple, std::index_sequence<Index...>>
{
    template <size_t Idx> static constexpr size_t NEXT_IDX = Idx + sizeof...(Index);

    template <bool Unsafe>
    static Tuple Get(StateRef vm, int idx)
    { return {vm.Get<TupleElement<Tuple, Index>, Unsafe>(idx+Index)...}; }

    static bool Is(StateRef vm, int idx)
    { return (vm.Is<TupleElement<Tuple, Index>>(idx+Index) && ...); }

    using type = brigand::list<TupleElement<Tuple, Index>...>;
    // IS: should be called on underlying types
};

template <typename T>
struct GetArg<T, EnableIfTupleLike<std::decay_t<T>>>
    : TupleGet<std::decay_t<T>,
               std::make_index_sequence<TupleLike<std::decay_t<T>>::SIZE>> {};

template <bool Unsafe, size_t N, typename Seq, typename... Args>
struct GenArgSequence;
template <bool Unsafe, size_t N, size_t... Seq, typename Head, typename... Args>
struct GenArgSequence<Unsafe, N, std::index_sequence<Seq...>, Head, Args...>
{
    using Type = typename GenArgSequence<
        Unsafe,
        GetArg<Head>::template NEXT_IDX<N>,
        std::index_sequence<Seq..., N>,
        Args...>::Type;
};
template <bool Unsafe, size_t N, typename Seq>
struct GenArgSequence<Unsafe, N, Seq>
{ using Type = Seq; };


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
        NEPTOOLS_UNREACHABLE("lua_error returned");
    }
}

template <auto Fun, bool Unsafe, typename Ret, typename Args, typename Seq>
struct WrapFunGen;

template <auto Fun, bool Unsafe, typename Ret, typename... Args, size_t... Seq>
struct WrapFunGen<Fun, Unsafe, Ret, brigand::list<Args...>,
                  std::index_sequence<Seq...>>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        return ResultPush<Ret>::Push(
            vm, CatchInvoke(vm, Fun, GetArg<Args>::template Get<Unsafe>(vm, Seq)...));
    }
};

template <auto Fun, bool Unsafe, typename... Args, size_t... Seq>
struct WrapFunGen<Fun, Unsafe, void, brigand::list<Args...>,
                  std::index_sequence<Seq...>>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        CatchInvoke(vm, Fun, GetArg<Args>::template Get<Unsafe>(vm, Seq)...);
        return 0;
    }
};

template <auto Fun, bool Unsafe, typename Args> struct WrapFunGen2;
template <auto Fun, bool Unsafe, typename... Args>
struct WrapFunGen2<Fun, Unsafe, brigand::list<Args...>>
    : public WrapFunGen<
        Fun, Unsafe,
        FunctionReturn<decltype(Fun)>, brigand::list<Args...>,
        typename GenArgSequence<Unsafe, 1, std::index_sequence<>, Args...>::Type>
{};

template <auto Fun, bool Unsafe>
struct WrapFunc : WrapFunGen2<Fun, Unsafe, FunctionArguments<decltype(Fun)>>
{};

// allow plain old lua functions
template <int (*Fun)(lua_State*), bool Unsafe>
struct WrapFunc<Fun, Unsafe>
{ static constexpr const auto Func = Fun; };


// overload
template <typename Args, typename Seq> struct OverloadCheck2;
template <typename... Args, size_t... Seq>
struct OverloadCheck2<brigand::list<Args...>, std::index_sequence<Seq...>>
{
    static bool Is(StateRef vm)
    {
        (void) vm;
        return (GetArg<Args>::Is(vm, Seq) && ...);
    }

    template <typename ValsList>
    static constexpr bool IS = (
        GetArg<Args>::template IS<brigand::at_c<ValsList, Seq-1>> && ...);
};

template <typename Args> struct OverloadCheck;
template <typename... Args>
struct OverloadCheck<brigand::list<Args...>>
    : public OverloadCheck2<
        brigand::list<Args...>,
        typename GenArgSequence<true, 1, std::index_sequence<>, Args...>::Type>
{};

template <auto... Args> struct OverloadWrap;
template <auto Fun, auto... Rest>
struct OverloadWrap<Fun, Rest...>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        if (OverloadCheck<FunctionArguments<decltype(Fun)>>::Is(vm))
            return WrapFunc<Fun, true>::Func(vm);
        else
            return OverloadWrap<Rest...>::Func(vm);
    }
};

template<> struct OverloadWrap<>
{
    static int Func(lua_State* l)
    {
        return luaL_error(l, "Invalid arguments to overloaded function");
    }
};

#ifdef NEPTOOLS_LUA_OVERLOAD_CHECK
namespace b = brigand;

template <typename List> struct LCartesian;
template <typename List>
using Cartesian = typename LCartesian<List>::type;

template <typename Head, typename... Tail>
struct LCartesian<b::list<Head, Tail...>>
{
    using Rest = Cartesian<b::list<Tail...>>;
    using type = b::join<b::transform<
        Head,
        b::bind<b::transform,
                b::pin<Rest>,
                b::defer<b::bind<b::push_front, b::_1, b::parent<b::_1>>>>>>;
};
template <typename... T> struct LCartesian<b::list<b::list<T...>>>
{ using type = b::list<b::list<T>...>; };
template <> struct LCartesian<b::list<>> { using type = b::list<>; };

template <typename A, typename B>
using IsOverload = b::integral_constant<bool, OverloadCheck<A>::template IS<B>>;

// brigand as_set fails on duplicate elements
template <typename T>
using ToSet = b::fold<T, b::set<>, b::bind<b::insert, b::_state, b::_element>>;

// used to report overload problems in a user-friendly.. khgrrr.. way
template <typename CalculatedSize, typename ExpectedSize,
          typename CalledOverloads, typename Overloads>
constexpr inline void InvalidOverload() = delete;

template <auto... Overloads>
inline constexpr void CheckUnique()
{
    using Args = b::transform<
        b::list<decltype(Overloads)...>,
        b::bind<b::join,
                b::bind<b::transform,
                        b::bind<FunctionArguments, b::_1>,
                        b::defer<GetArg<b::_1>>>>>;

    // get possible argument types. add nil to simulate less arguments
    using ArgSet = ToSet<b::push_back<b::flatten<Args>, Raw<LUA_TNIL>>>;

    // longest argument count
    using MaxCount = b::fold<b::transform<Args, b::bind<b::size, b::_1>>,
                             b::size_t<0>, b::max<b::_1, b::_2>>;
    // all possible calls with argset
    using AllTests = Cartesian<
        b::filled_list<b::wrap<ArgSet, b::list>, MaxCount::value>>;

    // actually called function for each test
    using Check = b::defer<b::bind<IsOverload, b::_1, b::parent<b::_1>>>;
    using Called = b::transform<AllTests, b::bind<b::index_if, b::pin<Args>, Check>>;
    // set of called overloads
    using CalledSet = b::erase<ToSet<Called>, b::no_such_type_>;

    if constexpr (b::size<CalledSet>::value != sizeof...(Overloads))
        InvalidOverload<b::size<CalledSet>, b::size_t<sizeof...(Overloads)>,
                        Called, Args>();
}
#endif

}

template <bool DoCheck, auto... Funs>
inline void StateRef::PushFunction_()
{
    if constexpr (sizeof...(Funs) == 1)
        lua_pushcfunction(vm, (Detail::WrapFunc<Funs..., false>::Func));
    else
    {
#ifdef NEPTOOLS_LUA_OVERLOAD_CHECK
        if constexpr (DoCheck)
            Detail::CheckUnique<Funs...>();
#endif
        lua_pushcfunction(vm, (Detail::OverloadWrap<Funs...>::Func));
    }
}

}

#endif
#endif

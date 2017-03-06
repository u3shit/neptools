#ifndef UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#define UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#pragma once

#include "function_call_types.hpp"
#include "type_traits.hpp"

#ifdef NEPTOOLS_LUA_OVERLOAD_CHECK
#include <boost/hana/cartesian_product.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/maximum.hpp>
#include <boost/hana/minus.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/replicate.hpp>
#include <boost/hana/remove.hpp>
#include <boost/hana/set.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
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

template <typename... Args> struct List
{
#ifdef NEPTOOLS_LUA_OVERLOAD_CHECK
    static constexpr const auto ToHana = boost::hana::tuple_t<Args...>;
#endif
};

template <size_t I, typename Args> struct Get;
template <typename H, typename... Tail> struct Get<0, List<H, Tail...>>
{ using Type = H; };
template <size_t I, typename H, typename... Tail> struct Get<I, List<H, Tail...>>
{ using Type = typename Get<I-1, List<Tail...>>::Type; };

template <typename T, typename Enable = void> struct GetArg;

template <typename List, typename... Res> struct Flatten1;
template <typename... First, typename... Second, typename... Res>
struct Flatten1<List<List<First...>, Second...>, Res...>
{ using Type = typename Flatten1<List<Second...>, Res..., First...>::Type; };

template <typename... Res> struct Flatten1<List<>, Res...>
{ using Type = List<Res...>; };


template <typename T> struct FunctionTraits;
template <typename Ret, typename... Args> struct FunctionTraits<Ret(Args...)>
{
    using Return = Ret;
    using Arguments = List<Args...>;
    using ArgumentTypes = typename Flatten1<List<typename GetArg<Args>::Type...>>::Type;
};

template <typename Ret, typename... Args>
struct FunctionTraits<Ret(*)(Args...)> : FunctionTraits<Ret(Args...)> {};
template <typename Ret, typename C, typename... Args>
struct FunctionTraits<Ret(C::*)(Args...)> : FunctionTraits<Ret(C&, Args...)> {};
template <typename Ret, typename C, typename... Args>
struct FunctionTraits<Ret(C::*)(Args...) const> : FunctionTraits<Ret(C&, Args...)> {};

#if __cpp_noexcept_function_type >= 201510
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(*)(Args...) noexcept> : FunctionTraits<Ret(Args...)> {};
template <typename Ret, typename C, typename... Args>
struct FunctionTraits<Ret(C::*)(Args...) noexcept> : FunctionTraits<Ret(C&, Args...)> {};
template <typename Ret, typename C, typename... Args>
struct FunctionTraits<Ret(C::*)(Args...) const noexcept> : FunctionTraits<Ret(C&, Args...)> {};
#endif

template <typename T, typename> struct GetArg
{
    using RawType = typename std::decay<T>::type;
    using Type = List<RawType>;

    template <int Idx> static constexpr size_t NEXT_IDX = Idx+1;

    template <bool Unsafe> static decltype(auto) Get(StateRef vm, int idx)
    { return Unsafe ? vm.UnsafeGet<RawType>(idx) : vm.Check<RawType>(idx); }
    static bool Is(StateRef vm, int idx) { return vm.Is<RawType>(idx); }

    template <typename Val>
    static constexpr const bool IS = COMPATIBLE_WITH<RawType, Val>;
};

template <> struct GetArg<Skip>
{
    template <int Idx> static constexpr size_t NEXT_IDX = Idx+1;
    template <bool>
    static constexpr Skip Get(StateRef, int) noexcept { return {}; }
    static constexpr bool Is(StateRef, int) noexcept { return true; }

    using Type = List<>;
    template <typename Val>
    static constexpr const bool IS = true;
};

template <> struct GetArg<StateRef>
{
    template <int Idx> static constexpr size_t NEXT_IDX = Idx;
    template <bool>
    static constexpr StateRef Get(StateRef vm, int) noexcept { return vm; }
    static constexpr bool Is(StateRef, int) noexcept { return true; }

    using Type = List<>;
    template <typename Val>
    static constexpr const bool IS = true;
};

template <int LType> struct GetArg<Raw<LType>>
{
    template <int Idx> static constexpr size_t NEXT_IDX = Idx+1;
    template <bool Unsafe>
    static Raw<LType> Get(StateRef vm, int idx)
    {
        if (!Unsafe && !Is(vm, idx))
            vm.TypeError(true, lua_typename(vm, LType), idx);
        return {};
    }
    static bool Is(StateRef vm, int idx) noexcept
    { return lua_type(vm, idx) == LType; }

    using Type = List<Raw<LType>>;
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
    template <int Idx> static constexpr size_t NEXT_IDX = Idx + sizeof...(Index);

    template <bool Unsafe>
    static Tuple Get(StateRef vm, int idx)
    {
        if constexpr (Unsafe)
            return {vm.UnsafeGet<TupleElement<Tuple, Index>>(idx+Index)...};
        else
            return {vm.Get<TupleElement<Tuple, Index>>(idx+Index)...};
    }
    static bool Is(StateRef vm, int idx)
    { return (vm.Is<TupleElement<Tuple, Index>>(idx+Index) && ...); }

    using Type = List<TupleElement<Tuple, Index>...>;
    // IS: should be called on underlying types
};

template <typename T>
struct GetArg<T, EnableIfTupleLike<T>>
    : TupleGet<T, std::make_index_sequence<TupleLike<T>::SIZE>> {};

template <bool Unsafe, int N, typename Seq, typename... Args>
struct GenArgSequence;
template <bool Unsafe, int N, int... Seq, typename Head, typename... Args>
struct GenArgSequence<Unsafe, N, std::integer_sequence<int, Seq...>, Head, Args...>
{
    using Type = typename GenArgSequence<
        Unsafe,
        GetArg<Head>::template NEXT_IDX<N>,
        std::integer_sequence<int, Seq..., N>,
        Args...>::Type;
};
template <bool Unsafe, int N, typename Seq> struct GenArgSequence<Unsafe, N, Seq>
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

template<typename T> struct ResultPush<T, EnableIfTupleLike<T>>
    : TuplePush<T, std::make_index_sequence<TupleLike<T>::SIZE>> {};

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

template <typename T, T Fun, bool Unsafe, typename Ret, typename Args, typename Seq>
struct WrapFunGen;

template <typename T, T Fun, bool Unsafe, typename Ret, typename... Args, int... Seq>
struct WrapFunGen<T, Fun, Unsafe, Ret, List<Args...>, std::integer_sequence<int, Seq...>>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        return ResultPush<Ret>::Push(
            vm, CatchInvoke(vm, Fun, GetArg<Args>::template Get<Unsafe>(vm, Seq)...));
    }
};

template <typename T, T Fun, bool Unsafe, typename... Args, int... Seq>
struct WrapFunGen<T, Fun, Unsafe, void, List<Args...>, std::integer_sequence<int, Seq...>>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        CatchInvoke(vm, Fun, GetArg<Args>::template Get<Unsafe>(vm, Seq)...);
        return 0;
    }
};

template <typename T, T Fun, bool Unsafe, typename Args> struct WrapFunGen2;
template <typename T, T Fun, bool Unsafe, typename... Args>
struct WrapFunGen2<T, Fun, Unsafe, List<Args...>>
    : public WrapFunGen<
        T, Fun, Unsafe,
        typename FunctionTraits<T>::Return, List<Args...>,
        typename GenArgSequence<Unsafe, 1, std::integer_sequence<int>, Args...>::Type>
{};

template <typename T, T Fun, bool Unsafe>
struct WrapFunc : WrapFunGen2<T, Fun, Unsafe, typename FunctionTraits<T>::Arguments>
{};

// allow plain old lua functions
template <int (*Fun)(lua_State*), bool Unsafe>
struct WrapFunc<int (*)(lua_State*), Fun, Unsafe>
{ static constexpr const auto Func = Fun; };


// overload
template <typename Args, typename Seq> struct OverloadCheck2;
template <typename... Args, int... Seq>
struct OverloadCheck2<List<Args...>, std::integer_sequence<int, Seq...>>
{
    static bool Is(StateRef vm)
    {
        (void) vm;
        return (GetArg<Args>::Is(vm, Seq) && ...);
    }

    template <typename ValsList>
    static constexpr bool IS = (
        GetArg<Args>::template IS<typename Get<Seq-1, ValsList>::Type> && ...);
};

template <typename Args> struct OverloadCheck;
template <typename... Args>
struct OverloadCheck<List<Args...>>
    : public OverloadCheck2<
        List<Args...>,
        typename GenArgSequence<true, 1, std::integer_sequence<int>, Args...>::Type>
{};

template <typename... Args> struct OverloadWrap;
template <typename T, T Fun, typename... Rest>
struct OverloadWrap<Overload<T, Fun>, Rest...>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        if (OverloadCheck<typename FunctionTraits<T>::Arguments>::Is(vm))
            return WrapFunc<T, Fun, true>::Func(vm);
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
namespace h = boost::hana;
struct GetArgs
{
    template <typename T, T Fun>
    inline constexpr auto operator()(h::basic_type<Overload<T, Fun>>)
    {
        return FunctionTraits<T>::ArgumentTypes::ToHana;
    }
};

template <typename T> struct ToList;
template <typename... Args> struct ToList<h::tuple<Args...>>
{ using Type = List<typename Args::type...>; };

// http://stackoverflow.com/a/33987589
template <typename Iterable, typename Pred>
constexpr auto index_of(const Iterable& iterable, Pred p)
{
    auto size = decltype(h::size(iterable)){};
    auto dropped = decltype(h::size(
        h::drop_while(iterable, p)
    )){};
    return size - dropped;
}

template <typename Test>
struct Check
{
    template <typename Overload>
    constexpr auto operator()(const Overload&) const
    {
        // warning: negated return
        return boost::hana::bool_c<
            !OverloadCheck<typename ToList<Overload>::Type>::
                template IS<typename ToList<Test>::Type>>;
    }
};

template <typename Overloads> struct Called
{
    Overloads& overloads;

    template <typename Test>
    constexpr auto operator()(const Test&) const
    {
        return index_of(overloads, Check<Test>{});
    }
};

// used to report overload problems in a user-friendly.. khgrrr.. way
template <typename CalculatedSize, typename ExpectedSize,
          typename CalledOverloads, typename Overloads>
constexpr inline void InvalidOverload(CalculatedSize, ExpectedSize,
                                      CalledOverloads, Overloads) = delete;

template <typename... Overloads>
inline constexpr void CheckUnique()
{
    constexpr auto overloads = h::transform(h::tuple_t<Overloads...>, GetArgs{});
    // get possible argument types. add nil to simulate less arguments
    constexpr auto argset = h::insert(
        h::to_set(h::flatten(overloads)), h::type_c<Raw<LUA_TNIL>>);
    // longest argument count
    constexpr auto max_count = h::maximum(h::transform(overloads, h::length));
    // all possible calls with argset
    constexpr auto all_test = h::cartesian_product(
        h::replicate<h::tuple_tag>(h::to_tuple(argset), max_count));

    // actually called function for each test
    constexpr auto called = h::transform(all_test, Called<decltype(overloads)>{overloads});
    // called index == sizeof...(Overloads) => failed to call (ignore)
    constexpr auto called_set = h::to_set(h::remove(called, h::size_c<sizeof...(Overloads)>));

    if constexpr (h::size(called_set) != sizeof...(Overloads))
    {
        InvalidOverload(h::size(called_set), h::size_c<sizeof...(Overloads)>,
                        called, overloads);
    }
}
#endif

}

template <typename T, T Fun>
inline void StateRef::Push()
{ lua_pushcfunction(vm, (Detail::WrapFunc<T, Fun, false>::Func)); }

template <typename Head, typename... Tail>
inline typename std::enable_if<IsOverload<Head>::value>::type StateRef::Push()
{
#ifdef NEPTOOLS_LUA_OVERLOAD_CHECK
    Detail::CheckUnique<Head, Tail...>();
#endif
    lua_pushcfunction(vm, (Detail::OverloadWrap<Head, Tail...>::Func));
}

}

#endif

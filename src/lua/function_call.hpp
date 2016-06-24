#ifndef UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#define UUID_C721F2E1_C293_4D82_8244_2AA0F1B26774
#pragma once

#include "type_traits.hpp"

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>

namespace Neptools
{
namespace Lua
{

// placeholder to skip parsing this argument
struct Skip {};

// the function pushes result manually
struct RetNum { int n; };

namespace Detail
{

template <typename... Args> struct List;

template <typename T> struct FunctionTraits;
template <typename Ret, typename... Args> struct FunctionTraits<Ret(Args...)>
{
    using Return = Ret;
    using Arguments = List<Args...>;
};

template <typename Ret, typename... Args>
struct FunctionTraits<Ret(*)(Args...)> : FunctionTraits<Ret(Args...)> {};
template <typename Ret, typename C, typename... Args>
struct FunctionTraits<Ret(C::*)(Args...)> : FunctionTraits<Ret(C*, Args...)> {};
template <typename Ret, typename C, typename... Args>
struct FunctionTraits<Ret(C::*)(Args...) const> : FunctionTraits<Ret(C*, Args...)> {};


template <typename T, int Idx> struct GetArg
{
    static constexpr size_t NEXT_IDX = Idx+1;
    static decltype(auto) Get(StateRef vm)
    { return vm.Check<typename std::decay<T>::type>(Idx); }
};

template <int Idx> struct GetArg<Skip, Idx>
{
    static constexpr size_t NEXT_IDX = Idx+1;
    static Skip Get(StateRef) { return {}; }
};

template <int Idx> struct GetArg<StateRef, Idx>
{
    static constexpr size_t NEXT_IDX = Idx;
    static StateRef Get(StateRef vm) { return vm; }
};

template <int N, typename Seq, typename... Args> struct GenArgSequence;
template <int N, int... Seq, typename Head, typename... Args>
struct GenArgSequence<N, std::integer_sequence<int, Seq...>, Head, Args...>
{
    using Type = typename GenArgSequence<
        GetArg<Head, N>::NEXT_IDX,
        std::integer_sequence<int, Seq..., N>,
        Args...>::Type;
};
template <int N, typename Seq> struct GenArgSequence<N, Seq>
{ using Type = Seq; };


template <typename T> struct ResultPush
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

template<typename... Args> struct ResultPush<std::tuple<Args...>>
{
    static int Push(StateRef vm, const std::tuple<Args...>& ret)
    {
        boost::fusion::for_each(
            ret, [=](auto&& p)
            {
                // well, it was reported only 9 years ago...
                // http://boost.2283326.n4.nabble.com/fusion-Stateful-unary-functor-for-for-each-loop-tp2565987p2565990.html
                const_cast<StateRef&>(vm).Push(std::forward<decltype(p)>(p));
            });
        return sizeof...(Args);
    }
};

// workaround gcc can't mangle noexcept template arguments...
template <typename... Args>
struct NothrowInvokable : std::integral_constant<
    bool, noexcept(Invoke(std::declval<Args>()...))> {};

template <typename... Args>
auto CatchInvoke(StateRef, Args&&... args) -> typename std::enable_if<
    NothrowInvokable<Args&&...>::value,
    decltype(Invoke(std::forward<Args>(args)...))>::type
{ return Invoke(std::forward<Args>(args)...); }

template <typename... Args>
auto CatchInvoke(StateRef vm, Args&&... args) -> typename std::enable_if<
    !NothrowInvokable<Args&&...>::value,
    decltype(Invoke(std::forward<Args>(args)...))>::type
{
    try { return Invoke(std::forward<Args>(args)...); }
    catch (const std::exception& e)
    {
        auto s = ExceptionToString();
        lua_pushlstring(vm, s.data(), s.size());
        lua_error(vm);
        NEPTOOLS_UNREACHABLE("lua_error returned");
    }
}

template <typename T, T Fun, typename Ret, typename Args, typename Seq>
struct WrapFunGen;

template <typename T, T Fun, typename Ret, typename... Args, int... Seq>
struct WrapFunGen<T, Fun, Ret, List<Args...>, std::integer_sequence<int, Seq...>>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        return ResultPush<Ret>::Push(
            vm, CatchInvoke(vm, Fun, GetArg<Args, Seq>::Get(vm)...));
    }
};

template <typename T, T Fun, typename... Args, int... Seq>
struct WrapFunGen<T, Fun, void, List<Args...>, std::integer_sequence<int, Seq...>>
{
    static int Func(lua_State* l)
    {
        StateRef vm{l};
        CatchInvoke(vm, Fun, GetArg<Args, Seq>::Get(vm)...);
        return 0;
    }
};

template <typename T, T Fun, typename Args> struct WrapFunGen2;
template <typename T, T Fun, typename... Args>
struct WrapFunGen2<T, Fun, List<Args...>>
    : public WrapFunGen<
        T, Fun, typename FunctionTraits<T>::Return, List<Args...>,
        typename GenArgSequence<1, std::integer_sequence<int>, Args...>::Type>
{};

template <typename T, T Fun>
using WrapFunc = WrapFunGen2<T, Fun, typename FunctionTraits<T>::Arguments>;

}

template <typename T, T Fun>
inline void StateRef::Push()
{ lua_pushcfunction(vm, (Detail::WrapFunc<T, Fun>::Func)); }

}
}

#endif

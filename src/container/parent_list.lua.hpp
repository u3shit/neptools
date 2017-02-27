#ifndef UUID_BCAE493F_27B2_4196_8980_F2289A849FED
#define UUID_BCAE493F_27B2_4196_8980_F2289A849FED
#pragma once

#include "parent_list.hpp"
#include "../lua/type_traits.hpp"
#include "../lua/function_ref.hpp"

namespace Neptools
{

template <typename Traits, bool IsConst>
struct Lua::TypeTraits<ParentListIterator<Traits, IsConst>>
{
    using Iterator = ParentListIterator<Traits, IsConst>;
    using RawType = typename Iterator::RawT;

    static Iterator Get(StateRef vm, bool arg, int idx)
    { return TypeTraits<RawType>::Get(vm, arg, idx); }

    static Iterator UnsafeGet(StateRef vm, int idx)
    { return TypeTraits<RawType>::UnsafeGet(vm, idx); }

    static bool Is(StateRef vm, int idx)
    { return TypeTraits<RawType>::Is(vm, idx); }

    static void Push(StateRef vm, Iterator it)
    { TypeTraits<RawType>::Push(vm, *it); }
};

template <typename T, typename LifetimeTraits = NullTraits,
          typename Traits = ParentListBaseHookTraits<T>>
struct ParentListLua
{
    using FakeClass = ParentList<T, LifetimeTraits, Traits>;

#define NEPTOOLS_GEN(name, op)                                      \
    static Lua::RetNum name(Lua::StateRef vm, FakeClass& pl, T& t)  \
    {                                                               \
        auto it = pl.template iterator_to<Check::Throw>(t);         \
        op it;                                                      \
        if (it == pl.end()) return 0;                               \
        else                                                        \
        {                                                           \
            vm.Push(*it);                                           \
            return 1;                                               \
        }                                                           \
    }
    NEPTOOLS_GEN(Next, ++)
    NEPTOOLS_GEN(Prev, --)
#undef NEPTOOLS_GEN

    static Lua::RetNum ToTable(Lua::StateRef vm, FakeClass& pl)
    {
        lua_createtable(vm, 0, 0);
        size_t i = 0;
        for (auto& it : pl)
        {
            vm.Push(it);
            lua_rawseti(vm, -2, i++);
        }
        return 1;
    }
};

}

#define NEPTOOLS_PARENT_LIST_LUAGEN(name, ...)                          \
    template struct ::Neptools::ParentListLua<__VA_ARGS__>;             \
    NEPTOOLS_LUA_TEMPLATE(name, nil, ::Neptools::ParentList<__VA_ARGS__>)

#endif

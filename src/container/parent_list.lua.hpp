#ifndef UUID_BCAE493F_27B2_4196_8980_F2289A849FED
#define UUID_BCAE493F_27B2_4196_8980_F2289A849FED
#pragma once

#ifdef NEPTOOLS_WITHOUT_LUA
#define NEPTOOLS_PARENT_LIST_LUAGEN(name, ...)
#else

#include "parent_list.hpp"
#include <libshit/lua/type_traits.hpp>
#include <libshit/lua/function_ref.hpp>

namespace Neptools
{

template <typename Traits, bool IsConst>
struct Lua::TypeTraits<ParentListIterator<Traits, IsConst>>
{
    using Iterator = ParentListIterator<Traits, IsConst>;
    using RawType = typename Iterator::RawT;

    template <bool Unsafe>
    static Iterator Get(StateRef vm, bool arg, int idx)
    { return TypeTraits<RawType>::template Get<Unsafe>(vm, arg, idx); }

    static bool Is(StateRef vm, int idx)
    { return TypeTraits<RawType>::Is(vm, idx); }

    static void Push(StateRef vm, Iterator it)
    { TypeTraits<RawType>::Push(vm, *it); }

    static void PrintName(std::ostream& os) { os << TYPE_NAME<RawType>; }
    static constexpr const char* TAG = TYPE_NAME<RawType>;
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

#define NEPTOOLS_PARENT_LIST_LUAGEN(name, cmp, ...)                     \
    template struct ::Neptools::ParentListLua<__VA_ARGS__>;             \
    NEPTOOLS_LUA_TEMPLATE(name, (comparable=cmp),                       \
                          ::Neptools::ParentList<__VA_ARGS__>)

#endif
#endif

#ifndef UUID_2DF4B391_2856_41E2_995A_53CABE00D614
#define UUID_2DF4B391_2856_41E2_995A_53CABE00D614
#pragma once

#include "ordered_map.hpp"
#include "container/ipairs.lua.h"
#include "../lua/function_call.hpp"

namespace Neptools
{

template <typename T, typename Traits,
          typename Compare = std::less<typename Traits::type>>
struct OrderedMapLua
{
    using FakeClass = OrderedMap<T, Traits, Compare>;

    static SmartPtr<T> get(OrderedMap<T, Traits, Compare>& om, size_t i)
    {
        if (i < om.size()) return &om[i];
        else return nullptr;
    }

    NEPTOOLS_LUAGEN(args={"/*$= class */&", "const typename /*$= class */::key_type&"})
    static SmartPtr<T> get(
        OrderedMap<T, Traits, Compare>& om,
        const typename OrderedMap<T, Traits, Compare>::key_type& key)
    {
        auto it = om.find(std::move(key));
        if (it == om.end()) return nullptr;
        else return &*it;
    }

    // todo: newindex -- what happens on key collission??
    // todo __ipairs: since lua 5.3, built-in ipairs calls metamethods

    // warning: return values swapped
    static std::tuple<bool, size_t> insert(
        OrderedMap<T, Traits, Compare>& om, size_t i,
        const NotNull<SmartPtr<T>>& t)
    {
        auto r = om.insert(om.checked_nth_end(i), t);
        return {r.second, om.index_of(r.first)};
    }

    static size_t erase(
        Lua::StateRef vm, OrderedMap<T, Traits, Compare>& om, size_t i, size_t e)
    {
        if (i > e) luaL_error(vm, "Invalid range");
        return om.index_of(om.erase(om.checked_nth_end(i), om.checked_nth_end(e)));
    }

    static size_t erase(
        OrderedMap<T, Traits, Compare>& om, size_t i)
    {
        return om.index_of(om.erase(om.checked_nth(i)));
    }

    // lua-compat: returns the erased value
    static NotNull<SmartPtr<T>> remove(
        OrderedMap<T, Traits, Compare>& om, size_t i)
    {
        auto it = om.checked_nth(i);
        NotNull<SmartPtr<T>> ret{&*it};
        om.erase(it);
        return ret;
    }

    // ret: pushed_back, index of old/new item
    static std::tuple<bool, size_t> push_back(
        OrderedMap<T, Traits, Compare>& om,
        const NotNull<SmartPtr<T>>& t)
    {
        auto r = om.push_back(t);
        return {r.second, om.index_of(r.first)};
    }

    // ret nil if not found
    // ret index, value if found
    NEPTOOLS_LUAGEN(args={"StateRef", "/*$= class */&", "const typename /*$= class */::key_type&"})
    static Lua::RetNum find(
        Lua::StateRef vm, OrderedMap<T, Traits, Compare>& om,
        const typename OrderedMap<T, Traits, Compare>::key_type& val)
    {
        auto r = om.find(val);
        if (r == om.end())
        {
            lua_pushnil(vm);
            return 1;
        }
        else
        {
            vm.Push(om.index_of(r));
            vm.Push(*r);
            return 2;
        }
    }

    static Lua::RetNum to_table(
        Lua::StateRef vm, OrderedMap<T, Traits, Compare>& om)
    {
        auto size = om.size();
        lua_createtable(vm, size ? size-1 : size, 0);
        for (size_t i = 0; i < size; ++i)
        {
            vm.Push(om[i]);
            lua_rawseti(vm, -2, i);
        }
        return 1;
    }
};
}

#define NEPTOOLS_ORDERED_MAP_LUAGEN(name, ...)                          \
    template struct ::Neptools::OrderedMapLua<__VA_ARGS__>;             \
    NEPTOOLS_LUA_TEMPLATE(name, ::Neptools::OrderedMap<__VA_ARGS__>)

#endif

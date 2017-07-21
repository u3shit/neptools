#ifndef UUID_2DF4B391_2856_41E2_995A_53CABE00D614
#define UUID_2DF4B391_2856_41E2_995A_53CABE00D614
#pragma once

#ifdef NEPTOOLS_WITHOUT_LUA
#define NEPTOOLS_ORDERED_MAP_LUAGEN(name, ...)
#else

#include "ordered_map.hpp"
#include "../lua/auto_table.hpp"
#include "../lua/function_call.hpp"

namespace Neptools
{

template <typename T, typename Traits,
          typename Compare = std::less<typename Traits::type>>
struct OrderedMapLua
{
    using FakeClass = OrderedMap<T, Traits, Compare>;

    static SmartPtr<T> get(OrderedMap<T, Traits, Compare>& om, size_t i) noexcept
    {
        if (i < om.size()) return &om[i];
        else return nullptr;
    }

    static SmartPtr<T> get(
        OrderedMap<T, Traits, Compare>& om,
        const typename OrderedMap<T, Traits, Compare>::key_type& key)
    {
        auto it = om.find(std::move(key));
        if (it == om.end()) return nullptr;
        else return &*it;
    }

    // ignore non-int/string keys
    static void get(OrderedMap<T, Traits, Compare>&, Lua::Skip) noexcept {}

    // todo: newindex -- what happens on key collission??
    // todo __ipairs: since lua 5.3, built-in ipairs calls metamethods

    // warning: return values swapped
    static std::tuple<bool, size_t> insert(
        OrderedMap<T, Traits, Compare>& om, size_t i,
        NotNull<SmartPtr<T>>&& t)
    {
        auto r = om.template insert<Check::Throw>(om.nth(i), std::move(t));
        return {r.second, om.index_of(r.first)};
    }

    static size_t erase(OrderedMap<T, Traits, Compare>& om, size_t i, size_t e)
    {
        return om.index_of(om.template erase<Check::Throw>(om.nth(i), om.nth(e)));
    }

    static size_t erase(
        OrderedMap<T, Traits, Compare>& om, size_t i)
    {
        return om.index_of(om.template erase<Check::Throw>(om.nth(i)));
    }

    // lua-compat: returns the erased value
    static NotNull<SmartPtr<T>> remove(
        OrderedMap<T, Traits, Compare>& om, size_t i)
    {
        auto it = om.checked_nth(i);
        NotNull<SmartPtr<T>> ret{&*it};
        om.template erase<Check::Throw>(it);
        return ret;
    }

    // ret: pushed_back, index of old/new item
    static std::tuple<bool, size_t> push_back(
        OrderedMap<T, Traits, Compare>& om,
        NotNull<SmartPtr<T>>&& t)
    {
        auto r = om.template push_back<Check::Throw>(std::move(t));
        return {r.second, om.index_of(r.first)};
    }

    // ret nil if not found
    // ret index, value if found
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

    NEPTOOLS_NOLUA static void FillFromTable(
        Lua::StateRef vm, OrderedMap<T, Traits, Compare>& om, Lua::RawTable tbl)
    {
        NEPTOOLS_LUA_GETTOP(vm, top);
        lua_rawgetp(vm, LUA_REGISTRYINDEX, Lua::TypeTraits<T>::TAG + 1); // +1
        auto newidx = lua_absindex(vm, -1);
        vm.Ipairs01(tbl, [&](size_t, int type)
        {
            if (type == LUA_TTABLE)
            {
                lua_pushvalue(vm, newidx);  // +1
                auto n = vm.Unpack01(lua_absindex(vm, -2)); // +1+n
                lua_call(vm, n, 1); // +1
                om.push_back(vm.Get<NotNull<SmartPtr<T>>>(-1));
                lua_pop(vm, 1); // 0
            }
            else
                om.push_back(vm.Get<NotNull<SmartPtr<T>>>(-1));
        });
        lua_pop(vm, 1);
        NEPTOOLS_LUA_CHECKTOP(vm, top);
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

// Can't copy an OrderedMap, so AutoTable<OrderedMap<...>> will only work with
// tables...
template <typename T, typename Traits, typename Compare>
struct Lua::TypeTraits<AT<OrderedMap<T, Traits, Compare>>>
{
    using RawType = OrderedMap<T, Traits, Compare>;
    template <bool Unsafe>
    static AT<RawType> Get(StateRef vm, bool arg, int idx)
    {
        if (!Unsafe && !Is(vm, idx))
            vm.TypeError(arg, "table", idx);
        AT<RawType> ret;
        OrderedMapLua<T, Traits, Compare>::FillFromTable(vm, ret, idx);
        return ret;
    }

    static bool Is(StateRef vm, int idx)
    { return lua_type(vm, idx) == LUA_TTABLE; }

    static void PrintName(std::ostream& os) { os << "table"; }
    static constexpr const char* TAG = TYPE_NAME<RawType>;
};

}

#define NEPTOOLS_ORDERED_MAP_LUAGEN(name, ...)                          \
    template struct ::Neptools::OrderedMapLua<__VA_ARGS__>;             \
    NEPTOOLS_LUA_TEMPLATE(name, (), ::Neptools::OrderedMap<__VA_ARGS__>)

#endif
#endif

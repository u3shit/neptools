#ifndef UUID_AFD13C49_2B38_4C98_88BE_F8D45F347F14
#define UUID_AFD13C49_2B38_4C98_88BE_F8D45F347F14
#pragma once

#include "shared_object.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace Neptools
{
namespace Lua
{

// lauxlib operations:
// luaL_check*: call lua_to*, fail if it fails
// luaL_opt*: lua_isnoneornil ? default : luaL_check*

template <typename T>
struct TypeTraits<T, std::enable_if_t<std::is_integral<T>::value>>
{
    static T Get(StateRef vm, bool arg, int idx)
    {
        int isnum;
        auto ret = lua_tointegerx(vm, idx, &isnum);
        if (BOOST_LIKELY(isnum)) return boost::numeric_cast<T>(ret);
        vm.TypeError(arg, "integer", idx);
    }

    static void Push(StateRef vm, T val)
    { lua_pushinteger(vm, val); }
};

template <typename T>
struct TypeTraits<T, std::enable_if_t<std::is_floating_point<T>::value>>
{
    static T Get(StateRef vm, bool arg, int idx)
    {
        int isnum;
        auto ret = lua_tonumberx(vm, idx, &isnum);
        if (BOOST_LIKELY(isnum)) return boost::numeric_cast<T>(ret);
        vm.TypeError(arg, "number", idx);
    }

    static void Push(StateRef vm, T val)
    { lua_pushnumber(vm, val); }
};

template <>
struct TypeTraits<bool>
{
    static bool Get(StateRef vm, bool arg, int idx)
    {
        if (BOOST_LIKELY(lua_isboolean(vm, idx)))
            return lua_toboolean(vm, idx);
        vm.TypeError(arg, "boolean", idx);
    }

    static void Push(StateRef vm, bool val)
    { lua_pushboolean(vm, val); }
};

template<>
struct TypeTraits<const char*>
{
    static const char* Get(StateRef vm, bool arg, int idx)
    {
        auto str = lua_tostring(vm, idx);
        if (BOOST_LIKELY(!!str)) return str;
        vm.TypeError(arg, "string", idx);
    };

    static void Push(StateRef vm, const char* val)
    { lua_pushstring(vm, val); }
};

template<>
struct TypeTraits<std::string>
{
    static std::string Get(StateRef vm, bool arg, int idx)
    {
        size_t len;
        auto str = lua_tolstring(vm, idx, &len);
        if (BOOST_LIKELY(!!str)) return {str, len};
        vm.TypeError(arg, "string", idx);
    };

    static void Push(StateRef vm, const std::string& val)
    { lua_pushlstring(vm, val.data(), val.length()); }
};

template<>
struct TypeTraits<std::shared_ptr<SharedObject>>
{
    template <typename T>
    static std::shared_ptr<SharedObject>& Get2(StateRef vm, bool arg, int idx)
    {
        {
            if (!lua_getmetatable(vm, idx)) goto fail; // +1
            lua_rawgetp(vm, -1, &T::TYPE_TAG); // +2
            auto nil = lua_isnil(vm, -1);
            lua_pop(vm, 2); // 0
            if (nil) goto fail;

            auto ptr = reinterpret_cast<std::shared_ptr<SharedObject>*>(
                lua_touserdata(vm, idx));
            return *ptr;
        }
    fail:
        vm.TypeError(arg, T::TYPE_NAME, idx);
    }

    static std::shared_ptr<SharedObject>& Get(StateRef vm, bool arg, int idx)
    { return Get2<SharedObject>(vm, arg, idx); }

    static void Push(StateRef vm, const std::shared_ptr<SharedObject>& ptr)
    {
        if (ptr) ptr->PushLua(vm);
        else lua_pushnil(vm);
    }

};

template <typename T>
struct TypeTraits<std::shared_ptr<T>,
                  std::enable_if_t<std::is_base_of<SharedObject, T>::value>>
    : public TypeTraits<std::shared_ptr<SharedObject>>
{
    static std::shared_ptr<T> Get(StateRef vm, bool arg, int idx)
    { return Get2<T>(vm, arg, idx); }
};

}
}

#endif

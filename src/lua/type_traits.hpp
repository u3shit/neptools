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
struct TypeTraits<T, std::enable_if_t<
    std::is_integral<T>::value || std::is_enum<T>::value>>
{
    static T Get(StateRef vm, bool arg, int idx)
    {
        int isnum;
        auto ret = lua_tointegerx(vm, idx, &isnum);
        if (BOOST_LIKELY(isnum)) return boost::numeric_cast<T>(ret);
        vm.TypeError(arg, "integer", idx);
    }

    static T UnsafeGet(StateRef vm, int idx)
    { return boost::numeric_cast<T>(lua_tointeger(vm, idx)); }

    static bool Is(StateRef vm, int idx)
    { return lua_isnumber(vm, idx); }

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

    static T UnsafeGet(StateRef vm, int idx)
    { return boost::numeric_cast<T>(lua_tonumber(vm, idx)); }

    static bool Is(StateRef vm, int idx)
    { return lua_isnumber(vm, idx); }

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

    static bool UnsafeGet(StateRef vm, int idx)
    { return lua_toboolean(vm, idx); }

    static bool Is(StateRef vm, int idx)
    { return lua_isboolean(vm, idx); }

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

    static const char* UnsafeGet(StateRef vm, int idx)
    { return lua_tostring(vm, idx); }

    static bool Is(StateRef vm, int idx)
    { return lua_isstring(vm, idx); }

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

    static std::string UnsafeGet(StateRef vm, int idx)
    {
        size_t len;
        auto str = lua_tolstring(vm, idx, &len);
        return {str, len};
    }

    static bool Is(StateRef vm, int idx)
    { return lua_isstring(vm, idx); }

    static void Push(StateRef vm, const std::string& val)
    { lua_pushlstring(vm, val.data(), val.length()); }
};

template <typename T>
struct TypeTraits<std::shared_ptr<T>,
                  std::enable_if_t<std::is_base_of<SharedObject, T>::value>>
{
    static std::shared_ptr<SharedObject>& Get_(StateRef vm, int idx)
    {
        auto ptr = reinterpret_cast<std::shared_ptr<SharedObject>*>(
            lua_touserdata(vm, idx));
        return *ptr;
    }

    template <typename U = T,
              typename = std::enable_if_t<std::is_same<U, SharedObject>::value>>
    static std::shared_ptr<SharedObject>& Get(StateRef vm, bool arg, int idx)
    {
        if (!Is(vm, idx))
            vm.TypeError(arg, T::TYPE_NAME, idx);
        return Get_(vm, idx);
    }

    template <typename U = T,
              typename = std::enable_if_t<!std::is_same<U, SharedObject>::value>>
    static std::shared_ptr<T> Get(StateRef vm, bool arg, int idx)
    {
        if (!Is(vm, idx))
            vm.TypeError(arg, T::TYPE_NAME, idx);
        return std::static_pointer_cast<T>(Get_(vm, idx));
    }

    template <typename U = T,
              typename = std::enable_if_t<std::is_same<U, SharedObject>::value>>
    static std::shared_ptr<SharedObject>& UnsafeGet(StateRef vm, int idx)
    { return Get_(vm, idx); }

    template <typename U = T,
              typename = std::enable_if_t<!std::is_same<U, SharedObject>::value>>
    static std::shared_ptr<T> UnsafeGet(StateRef vm, int idx)
    { return std::static_pointer_cast<T>(Get_(vm, idx)); }

    static bool Is(StateRef vm, int idx)
    {
        if (!lua_getmetatable(vm, idx)) return false; // +1
        auto type = lua_rawgetp(vm, -1, &T::TYPE_TAG); // +2
        lua_pop(vm, 2);
        return !IsNoneOrNil(type);
    }

    static void Push(StateRef vm, const std::shared_ptr<SharedObject>& ptr)
    {
        if (ptr) ptr->PushLua(vm);
        else lua_pushnil(vm);
    }
};

template <typename T>
struct TypeTraits<T*, std::enable_if_t<std::is_base_of<SharedObject, T>::value>>
{
    static T* Get(StateRef vm, bool arg, int idx)
    { return TypeTraits<std::shared_ptr<T>>::Get(vm, arg, idx).get(); }

    static T* UnsafeGet(StateRef vm, int idx)
    { return TypeTraits<std::shared_ptr<T>>::UnsafeGet(vm, idx).get(); }

    static bool Is(StateRef vm, int idx)
    { return TypeTraits<std::shared_ptr<T>>::Check(vm, idx); }
};

}
}

#endif

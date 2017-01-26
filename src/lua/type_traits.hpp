#ifndef UUID_AFD13C49_2B38_4C98_88BE_F8D45F347F14
#define UUID_AFD13C49_2B38_4C98_88BE_F8D45F347F14
#pragma once

#include "base.hpp"
#include "../nonowning_string.hpp"
#include <boost/numeric/conversion/cast.hpp>

namespace boost { namespace filesystem { class path; } }

namespace Neptools
{
template <typename T> class NotNull;

namespace Lua
{

// type name
template <typename T, typename Enable = void> struct TypeName
{ static constexpr const char* TYPE_NAME = T::TYPE_NAME; };

template <typename T>
struct TypeName<T, std::enable_if_t<
    std::is_integral<T>::value || std::is_enum<T>::value>>
{ static constexpr const char* TYPE_NAME = "integer"; };

template <typename T>
struct TypeName<T, std::enable_if_t<std::is_floating_point<T>::value>>
{ static constexpr const char* TYPE_NAME = "number"; };

template<> struct TypeName<bool>
{ static constexpr const char* TYPE_NAME = "boolean"; };

template<> struct TypeName<const char*>
{ static constexpr const char* TYPE_NAME = "string"; };
template<> struct TypeName<std::string>
{ static constexpr const char* TYPE_NAME = "string"; };

template <typename T>
constexpr const char* TYPE_NAME = TypeName<T>::TYPE_NAME;

#define NEPTOOLS_LUA_CLASS         \
    public:                        \
    static const char TYPE_NAME[]

// type tag
template <typename T> char TYPE_TAG = {};

// lauxlib operations:
// luaL_check*: call lua_to*, fail if it fails
// luaL_opt*: lua_isnoneornil ? default : luaL_check*

template <typename T> struct IsBoostEndian : std::false_type {};

template <typename T>
struct TypeTraits<T, std::enable_if_t<
    std::is_integral<T>::value || std::is_enum<T>::value ||
    IsBoostEndian<T>::value>>
{
    static T Get(StateRef vm, bool arg, int idx)
    {
        int isnum;
        auto ret = lua_tointegerx(vm, idx, &isnum);
        if (BOOST_LIKELY(isnum)) return boost::numeric_cast<T>(ret);
        vm.TypeError(arg, TYPE_NAME<T>, idx);
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
        vm.TypeError(arg, TYPE_NAME<T>, idx);
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
        vm.TypeError(arg, TYPE_NAME<bool>, idx);
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
        vm.TypeError(arg, TYPE_NAME<const char*>, idx);
    };

    static const char* UnsafeGet(StateRef vm, int idx)
    { return lua_tostring(vm, idx); }

    static bool Is(StateRef vm, int idx)
    { return lua_isstring(vm, idx); }

    static void Push(StateRef vm, const char* val)
    { lua_pushstring(vm, val); }
};

template <typename T>
struct TypeTraits<T, std::enable_if_t<
    std::is_same<T, std::string>::value ||
    std::is_same<T, NonowningString>::value ||
    std::is_same<T, StringView>::value>>
{
    static T Get(StateRef vm, bool arg, int idx)
    {
        size_t len;
        auto str = lua_tolstring(vm, idx, &len);
        if (BOOST_LIKELY(!!str)) return T(str, len);
        vm.TypeError(arg, TYPE_NAME<std::string>, idx);
    };

    static T UnsafeGet(StateRef vm, int idx)
    {
        size_t len;
        auto str = lua_tolstring(vm, idx, &len);
        return T(str, len);
    }

    static bool Is(StateRef vm, int idx)
    { return lua_isstring(vm, idx); }

    static void Push(StateRef vm, const T& val)
    { lua_pushlstring(vm, val.data(), val.length()); }
};

template<>
struct TypeTraits<boost::filesystem::path> : public TypeTraits<const char*>
{
    template <typename T> // T will be boost::filesystem::path, but it's only
                          // fwd declared at the moment...
    static void Push(StateRef vm, const T& pth)
    {
#ifdef WINDOWS
        auto str = pth.string();
        lua_pushlstring(vm, str.c_str(), str.size());
#else
        lua_pushlstring(vm, pth.c_str(), pth.size());
#endif
    }
};

template <typename T, typename Ret = T>
struct NullableTypeTraits
{
    static Ret Get(StateRef vm, bool arg, int idx)
    {
        if (lua_isnil(vm, idx)) return nullptr;
        return TypeTraits<NotNull<T>>::Get(vm, arg, idx);
    }

    static Ret UnsafeGet(StateRef vm, int idx)
    {
        if (lua_isnil(vm, idx)) return nullptr;
        return TypeTraits<NotNull<T>>::UnsafeGet(vm, idx);
    }

    static bool Is(StateRef vm, int idx)
    { return lua_isnil(vm, idx) || TypeTraits<NotNull<T>>::Is(vm, idx); }

    static void Push(StateRef vm, const T& obj)
    {
        if (obj) TypeTraits<NotNull<T>>::Push(vm, NotNull<T>{obj});
        else lua_pushnil(vm);
    }
};

// used by UserType
template <typename T, typename Enable = void> struct UserTypeTraits;

}
}

#endif

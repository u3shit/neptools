#ifndef UUID_0DFFCD6C_CA34_4A76_B8D7_1A0B4DF20DC6
#define UUID_0DFFCD6C_CA34_4A76_B8D7_1A0B4DF20DC6
#pragma once

#include "type_traits.hpp"

namespace Neptools
{
namespace Lua
{

struct ValueObject {};

// specialize if needed
template <typename T>
struct IsValueObject : std::is_base_of<ValueObject, T> {};

template <typename T>
struct TypeTraits<T, std::enable_if_t<IsValueObject<T>::value>>
{
    static T& Get(StateRef vm, bool arg, int idx)
    {
        if (!Is(vm, idx))
            vm.TypeError(arg, TYPE_NAME<T>, idx);
        return UnsafeGet(vm, idx);
    }

    static T& UnsafeGet(StateRef vm, int idx)
    { return *reinterpret_cast<T*>(lua_touserdata(vm, idx)); }

    static bool Is(StateRef vm, int idx)
    {
        if (!lua_getmetatable(vm, idx)) return false; // +1
        auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &TYPE_TAG<T>); // +2
        NEPTOOLS_ASSERT(!IsNoneOrNil(type)); (void) type;
        auto ret = lua_rawequal(vm, -1, -2);
        lua_pop(vm, 1); // 0
        return ret;
    }

    template <typename... Args>
    static void Push(StateRef vm, Args&&... args)
    {
        auto ptr = lua_newuserdata(vm, sizeof(T)); // +1
        auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &TYPE_TAG<T>); // +2
        NEPTOOLS_ASSERT(!IsNoneOrNil(type)); (void) type;

        new (ptr) T{std::forward<Args>(args)...};
        lua_setmetatable(vm, -2); // +1
    }
};

template <typename T>
struct TypeTraits<T*, std::enable_if_t<IsValueObject<T>::value>>
{
    static T* Get(StateRef vm, bool arg, int idx)
    {
        if (!Is(vm, idx))
            vm.TypeError(arg, TYPE_NAME<T>, idx);
        return UnsafeGet(vm, idx);
    }

    static T* UnsafeGet(StateRef vm, int idx)
    { return reinterpret_cast<T*>(lua_touserdata(vm, idx)); }

    static bool Is(StateRef vm, int idx)
    { return TypeTraits<T>::Is(vm, idx); }
};

}
}

#endif

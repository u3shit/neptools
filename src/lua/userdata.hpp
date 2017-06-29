#ifndef UUID_A1233430_4156_4C5B_A938_774F7FB4A2A5
#define UUID_A1233430_4156_4C5B_A938_774F7FB4A2A5
#pragma once

#include "base.hpp"
#include "function_call_types.hpp"
#include "type_traits.hpp"

namespace Neptools::Lua::Userdata
{

void UnsetMetatable(StateRef vm);

bool IsSimple(StateRef vm, int idx, const char* name);

template <bool Unsafe, typename Ret>
Ret& GetSimple(StateRef vm, bool arg, int idx, const char* name)
{
    if (!Unsafe && !IsSimple(vm, idx, name))
        vm.TypeError(arg, name, idx);
    return *reinterpret_cast<Ret*>(lua_touserdata(vm, idx));
}

template <typename T, typename... Args>
inline RetNum Create(StateRef vm, Args&&... args)
{
    NEPTOOLS_LUA_GETTOP(vm, top);
    auto ptr = lua_newuserdata(vm, sizeof(T)); // +1
    NEPTOOLS_ASSERT(ptr);
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, TYPE_NAME<T>); // +2
    NEPTOOLS_ASSERT(!IsNoneOrNil(type)); (void) type;

    new (ptr) T{std::forward<Args>(args)...};
    lua_setmetatable(vm, -2); // +1
    NEPTOOLS_LUA_CHECKTOP(vm, top+1);
    return 1;
}

namespace Cached
{

template <typename T, typename... Args>
inline void Create(StateRef vm, void* ptr, const char* name, Args&&... args)
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    // check cache
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
    NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;
    type = lua_rawgetp(vm, -1, ptr); // +2
    if (type != LUA_TUSERDATA) // no hit
    {
        lua_pop(vm, 1); // +1

        // create object
        auto ud = lua_newuserdata(vm, sizeof(T)); // +1
        auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, name); // +2
        NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;

        new (ud) T{std::forward<Args>(args)...};
        lua_setmetatable(vm, -2); // +1

        // cache it
        lua_pushvalue(vm, -1); // +3
        lua_rawsetp(vm, -3, ptr); // +2
    }
    else
    {
        NEPTOOLS_ASSERT_MSG(
            lua_getmetatable(vm, -1) &&
            lua_rawgetp(vm, -1, name) == LUA_TNUMBER &&
            (lua_pop(vm, 2), true), "Pointer aliasing?");
    }

    lua_remove(vm, -2); // +1 remove reftbl
    NEPTOOLS_LUA_CHECKTOP(vm, top+1);
}

void Clear(StateRef vm, void* ptr);

template <typename T, const char* Tag>
inline int GcFun(lua_State* vm)
{
    NEPTOOLS_LUA_GETTOP(vm, top);
    if (!lua_getmetatable(vm, 1) || // +1
        IsNoneOrNil(lua_rawgetp(vm, -1, Tag))) // +2
        StateRef{vm}.TypeError(true, Tag, 1);
    lua_pop(vm, 2); // 0

    auto ud = reinterpret_cast<T*>(lua_touserdata(vm, 1));
    NEPTOOLS_ASSERT(ud);
    Clear(vm, &**ud);
    ud->~T();

    UnsetMetatable(vm);
    NEPTOOLS_LUA_CHECKTOP(vm, top);
    return 0;

}

}
}

#endif

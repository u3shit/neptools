#include "userdata.hpp"

namespace Neptools::Lua::Userdata
{

void UnsetMetatable(StateRef vm)
{
    lua_pushnil(vm); // +1
    lua_setmetatable(vm, 1); // 0
}

bool IsSimple(StateRef vm, int idx, const char* name)
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    if (!lua_getmetatable(vm, idx)) // +1|0
    {
        NEPTOOLS_LUA_CHECKTOP(vm, top);
        return false;
    }
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, name); // +2
    NEPTOOLS_ASSERT(!IsNoneOrNil(type)); (void) type;
    auto ret = lua_rawequal(vm, -1, -2);
    lua_pop(vm, 2); // 0

    NEPTOOLS_LUA_CHECKTOP(vm, top);
    return ret;
}

bool IsInherited(StateRef vm, int idx, const char* name)
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    if (!lua_getmetatable(vm, idx))  // +1|0
    {
        NEPTOOLS_LUA_CHECKTOP(vm, top);
        return false;
    }
    auto type = lua_rawgetp(vm, -1, name); // +2
    lua_pop(vm, 2); // 0

    NEPTOOLS_LUA_CHECKTOP(vm, top);
    return type == LUA_TNUMBER;
}

void Cached::Clear(StateRef vm, void* ptr)
{
    NEPTOOLS_LUA_GETTOP(vm, top);
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
    NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;
    lua_pushnil(vm); // +2
    lua_rawsetp(vm, -2, ptr); // +1, if this throws we're screwed
    lua_pop(vm, 1); // 0

    UnsetMetatable(vm);
    NEPTOOLS_LUA_CHECKTOP(vm, top);
}


}

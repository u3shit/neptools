#include "user_type.hpp"
#include <cstring>

namespace Neptools
{
namespace Lua
{

static int cont_gettop(lua_State* vm, int, lua_KContext ctx)
{
    return lua_gettop(vm) - ctx;
}

static int index(lua_State* vm)
{
    // indexes:
    // 1 -> self
    // 2 -> key
    // upval 1 -> metatable

    lua_pushvalue(vm, 2); // +1
    lua_rawget(vm, lua_upvalueindex(1)); // +1
    if (!lua_isnil(vm, -1)) return -1;
    lua_pop(vm, 1); // +0

    if (lua_isstring(vm, 2))
    {
        lua_pushfstring(vm, "get_%s", lua_tostring(vm, 2)); // +1
        lua_rawget(vm, lua_upvalueindex(1)); // +1
        if (!lua_isnil(vm, -1))
        {
            lua_pushvalue(vm, 1); // +2
            auto top = lua_gettop(vm);
            lua_callk(vm, 1, LUA_MULTRET, top-2, cont_gettop); // +0+
            return lua_gettop(vm) - top + 2;
        }
        lua_pop(vm, 1); // +0
    }

    lua_pushliteral(vm, "get"); // +1
    lua_rawget(vm, lua_upvalueindex(1)); // +1
    if (!lua_isnil(vm, -1))
    {
        lua_pushvalue(vm, 1); // +2
        lua_pushvalue(vm, 2); // +3
        auto top = lua_gettop(vm);
        lua_callk(vm, 2, LUA_MULTRET, top-3, cont_gettop); // +0+
        return lua_gettop(vm) - top + 3;
    }
    return 0;
}

static int newindex(lua_State* vm)
{
    // 1 -> userdata
    // 2 -> key
    // 3 -> value
    // upval 1 -> metatable
    if (lua_isstring(vm, 2))
    {
        lua_pushfstring(vm, "set_%s", lua_tostring(vm, 2)); // +1
        lua_rawget(vm, lua_upvalueindex(1)); // +1
        if (!lua_isnil(vm, -1))
        {
            lua_pushvalue(vm, 1); // +2
            auto top = lua_gettop(vm);
            lua_callk(vm, 2, LUA_MULTRET, top-3, cont_gettop); // +0+
            return lua_gettop(vm) - top + 3;
        }
        lua_pop(vm, 1); // +0
    }

    lua_pushliteral(vm, "set"); //+1
    lua_rawget(vm, lua_upvalueindex(1)); //+1
    if (!lua_isnil(vm, -1))
    {
        // user,key,value,...,fun => ...,fun,user,key,value
        lua_rotate(vm, 1, -3);
        auto top = lua_gettop(vm);
        lua_callk(vm, 3, LUA_MULTRET, top-4, cont_gettop);
        return lua_gettop(vm) - top + 4;
    }

    // bail out
    auto key = luaL_tolstring(vm, 2, nullptr);
    return luaL_error(vm, "attempt to set invalid key '%s'", key);
}


TypeBuilder::TypeBuilder(StateRef vm, void* type_tag) : vm{vm}
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    // type table
    lua_createtable(vm, 0, 3); // __index, __call, new at least usually
    lua_pushvalue(vm, -1);
    lua_setfield(vm, -2, "__index");
    lua_pushvalue(vm, -1);
    lua_setmetatable(vm, -2);

    // metatable
    lua_createtable(vm, 0, 5);
    lua_pushliteral(vm, "private");
    lua_setfield(vm, -2, "__metatable");

    lua_pushvalue(vm, -1);
    lua_setfield(vm, -2, "__index");

    lua_pushvalue(vm, -1);
    lua_rawsetp(vm, -2, type_tag);

    lua_pushvalue(vm, -1);
    lua_pushcclosure(vm, newindex, 1);
    lua_setfield(vm, -2, "__newindex");

    lua_pushvalue(vm, -1);
    lua_rawsetp(vm, LUA_REGISTRYINDEX, type_tag);

    NEPTOOLS_LUA_CHECKTOP(vm, top+2);
}

TypeBuilder::~TypeBuilder()
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    lua_pushnil(vm); // +1
    while (lua_next(vm, -2)) // +2
    {
        if (lua_isstring(vm, -2) && strncmp(lua_tostring(vm, -2), "get", 3) == 0)
        {
            lua_pushvalue(vm, -1); // +3
            lua_pushcclosure(vm, index, 1); // +3
            lua_setfield(vm, -4, "__index"); // +2
            lua_pop(vm, 2); // +0
            break;
        }
        lua_pop(vm, 1); // +1
    }

    lua_remove(vm, -2); // -1

    NEPTOOLS_LUA_CHECKTOP(vm, top-1);
}

TypeBuilder& TypeBuilder::Name(const char* name)
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    lua_pushstring(vm, name);
    lua_setfield(vm, -2, "__name");
    lua_pushvalue(vm, -1);
    lua_setglobal(vm, name);

    NEPTOOLS_LUA_CHECKTOP(vm, top);
    return *this;
}

void TypeBuilder::SetField(const char* name)
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    lua_pushvalue(vm, -1);
    lua_setfield(vm, -4, name);
    lua_setfield(vm, -2, name);

    NEPTOOLS_LUA_CHECKTOP(vm, top-1);
}

void TypeBuilder::DoInherit()
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    lua_pushnil(vm); // +1
    while (lua_next(vm, -2)) // +2
    {
        lua_pushvalue(vm, -2); // +3
        lua_rawget(vm, -5); // +3
        if (lua_isnil(vm, -1))
        {
            lua_pop(vm, 1); // +2
            lua_pushvalue(vm, -2); //+3
            lua_pushvalue(vm, -2); //+4
            lua_rawset(vm, -6); // +2
            lua_pop(vm, 1); // +1
        }
        else
            lua_pop(vm, 2); // +1
    }
    lua_pop(vm, 1); // -1

    NEPTOOLS_LUA_CHECKTOP(vm, top-1);
}

}
}

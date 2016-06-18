#include "user_type.hpp"
#include <cstring>

namespace Neptools
{
namespace Lua
{

static int index(lua_State* vm)
{
    // indexes:
    // 1 -> self
    // 2 -> key
    // upval 1 -> metatable

    lua_pushvalue(vm, 2); // +1
    if (lua_rawget(vm, lua_upvalueindex(1)) > 0) return 1; // +1
    lua_pop(vm, 1); // +0

    if (lua_isstring(vm, 2))
    {
        lua_pushfstring(vm, "get_%s", lua_tostring(vm, 2)); // +1
        if (lua_rawget(vm, lua_upvalueindex(1)) == LUA_TFUNCTION) // +1
        {
            lua_pushvalue(vm, 1); // +2
            lua_call(vm, 1, 1); // +1
            return 1;
        }
        lua_pop(vm, 1); // +0
    }

    lua_pushliteral(vm, "get"); // +1
    if (lua_rawget(vm, lua_upvalueindex(1)) == LUA_TFUNCTION) // +1
    {
        lua_pushvalue(vm, 1); // +2
        lua_pushvalue(vm, 2); // +3
        lua_call(vm, 2, 1); // +1
        return 1;
    }
    return 0;
}

static int newindex(lua_State* vm)
{
    // 1 -> self
    // 2 -> key
    // 3 -> value
    // upval 1 -> metatable
    if (lua_isstring(vm, 2))
    {
        lua_pushfstring(vm, "set_%s", lua_tostring(vm, 2)); // +1
        if (lua_rawget(vm, lua_upvalueindex(1)) == LUA_TFUNCTION) // +1
        {
            lua_pushvalue(vm, 1); // +2
            lua_pushvalue(vm, 3); // +3
            lua_call(vm, 2, 1); // +1
            return 1;
        }
        lua_pop(vm, 1); // +0
    }

    lua_pushliteral(vm, "set"); //+1
    if (lua_rawget(vm, lua_upvalueindex(1)) == LUA_TFUNCTION) //+1
    {
        // self,key,value,...,fun => ...,fun,self,key,value
        lua_rotate(vm, 1, -3);
        lua_call(vm, 3, 1);
        return 1;
    }

    // bail out
    auto key = luaL_tolstring(vm, 2, nullptr);
    return luaL_error(vm, "attempt to set invalid key '%s'", key);
}


TypeBuilder::TypeBuilder(StateRef vm, void* type_tag) : vm{vm}
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    // type table
    lua_createtable(vm, 0, 0); // +1
    lua_getfield(vm, LUA_REGISTRYINDEX, "neptools_new_mt"); // +2
    NEPTOOLS_ASSERT(lua_istable(vm, -1));
    lua_setmetatable(vm, -2); // +1

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

void TypeBuilder::Done()
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    lua_pushnil(vm); // +1
    while (lua_next(vm, -2)) // +2/0 on end
    {
        if (lua_isstring(vm, -2) && strncmp(lua_tostring(vm, -2), "get", 3) == 0)
        {
            lua_pushvalue(vm, -3); // +3
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

    // type_table.__name = name
    lua_pushstring(vm, name); // +1
    lua_setfield(vm, -2, "__name"); // 0

    // set global name
    // local tbl = _G
    lua_pushglobaltable(vm); // +1
    const char* dot;
    while (dot = strchr(name, '.'))
    {
        // tbl = tbl[name_chunk] ||= {}
        // {
        // will be pushed again when subtable doesn't exists, but optimize for
        // common case where it already exists
        lua_pushlstring(vm, name, dot-name); // +2
        auto typ = lua_rawget(vm, -2); // +2
        if (typ <= 0) // no subtable, create it
        {
            lua_pop(vm, 1); // +1

            lua_createtable(vm, 0, 1); // +2 new tbl
            lua_pushlstring(vm, name, dot-name); // +3
            lua_pushvalue(vm, -2); // +4
            lua_rawset(vm, -4); // +2
        }

        lua_remove(vm, -2); // +1
        // }
        name = dot+1;
    }

    // tbl[name] = type_table
    lua_pushvalue(vm, -3); // +2
    lua_setfield(vm, -2, name); // +1
    lua_pop(vm, 1);

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
    while (lua_next(vm, -2)) // +2/0
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

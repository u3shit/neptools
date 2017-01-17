#include "user_type.hpp"
#include <cstring>

namespace Neptools
{
namespace Lua
{

TypeBuilder::TypeBuilder(StateRef vm, void* type_tag, const char* name)
    : vm{vm}
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

    lua_pushinteger(vm, 0);
    lua_rawsetp(vm, -2, type_tag);

    lua_pushvalue(vm, -1);
    lua_rawsetp(vm, LUA_REGISTRYINDEX, type_tag);

    // type_table.__name = name
    lua_pushstring(vm, name); // +1
    lua_setfield(vm, -2, "__name"); // 0

    // set global name
    lua_pushglobaltable(vm); // +1
    vm.SetRecTable(name, -3); // 0

    NEPTOOLS_LUA_CHECKTOP(vm, top+2);
}

static void SetMt(StateRef vm, const char* dst, const char* mt)
{
    lua_getfield(vm, LUA_REGISTRYINDEX, mt);
    NEPTOOLS_ASSERT(lua_isfunction(vm, -1));
    lua_pushvalue(vm, -2); // +2
    lua_call(vm, 1, 1); // +1
    lua_setfield(vm, -2, dst); // +0
}

void TypeBuilder::Done()
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    if (has_get_ || has_get)
        SetMt(
            vm, "__index",
            has_get_ ? "neptools_mt_index" : "neptools_mt_index_light");

    if (has_set_) SetMt(vm, "__newindex", "neptools_mt_newindex");
    else if (has_set)
    {
        lua_getfield(vm, -1, "set"); // +1
        NEPTOOLS_ASSERT(lua_isfunction(vm, -1));
        lua_setfield(vm, -2, "__newindex"); // +0
    }

    lua_remove(vm, -2);

    NEPTOOLS_LUA_CHECKTOP(vm, top-1);
}

void TypeBuilder::DoInherit(ptrdiff_t offs)
{
    NEPTOOLS_LUA_GETTOP(vm, top);
    // -1: base class meta
    // -2: this meta

    NEPTOOLS_ASSERT(lua_type(vm, -1) == LUA_TTABLE);

    // for k,v in pairs(base_mt) do
    lua_pushnil(vm); // +1
    while (lua_next(vm, -2)) // +2/0
    {
        // if not this_mt[k] then
        lua_pushvalue(vm, -2); // +3
        auto type = lua_rawget(vm, -5); // +3
        if (IsNoneOrNil(type))
        {
            lua_pop(vm, 1); // +2
            // this_mt[k] = if_below ? v+offs : v
            lua_pushvalue(vm, -2); //+3

            // if type(k) == "lightuserdata" and type(v) == "number"
            if (lua_islightuserdata(vm, -3) && lua_isnumber(vm, -2))
                lua_pushinteger(vm, lua_tointeger(vm, -2)+offs); // +4
            else
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

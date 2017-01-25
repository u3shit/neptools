#include "user_data.hpp"

namespace Neptools::Lua
{

void UserDataBase::ClearCache(StateRef vm) noexcept
{
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
    NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;
    lua_pushnil(vm); // +2
    lua_rawsetp(vm, -2, obj); // +1, if this throws we're screwed
    lua_pop(vm, 1); // 0
}

namespace UserDataDetail
{
char TAG;

void TypeTraits::GcFun(StateRef vm)
{
    if (!lua_getmetatable(vm, 1) || // +1
        IsNoneOrNil(lua_rawgetp(vm, -1, &TAG))) // +2
        vm.TypeError(true, "neptools.object", 1);
    lua_pop(vm, 2);

    auto ub = reinterpret_cast<UserDataBase*>(lua_touserdata(vm, 1));
    NEPTOOLS_ASSERT(ub);
    ub->Destroy(vm);

    lua_pushnil(vm);
    lua_setmetatable(vm, 1);
}

UBArgs GetBase(
    StateRef vm, bool arg, int idx, const char* name, void* tag)
{
    if (!lua_getmetatable(vm, idx)) // +1
        vm.TypeError(arg, name, idx);
    lua_rawgetp(vm, -1, tag); // +2

    int isvalid;
    auto offs = lua_tointegerx(vm, -1, &isvalid);
    lua_pop(vm, 2); // 0
    if (!isvalid) vm.TypeError(arg, name, idx);

    auto ud = static_cast<UserDataBase*>(lua_touserdata(vm, idx));
    NEPTOOLS_ASSERT(ud);
    return {ud, offs};
}

UBArgs UnsafeGetBase(StateRef vm, int idx, void* tag)
{
    lua_getmetatable(vm, idx); // +1
    lua_rawgetp(vm, -1, tag); // +2
    auto offs = lua_tointeger(vm, -1);
    lua_pop(vm, 2); // 0

    auto ud = static_cast<UserDataBase*>(lua_touserdata(vm, idx));
    NEPTOOLS_ASSERT(ud);
    return {ud, offs};
}

bool IsBase(StateRef vm, int idx, void* tag)
{
    if (!lua_getmetatable(vm, idx)) // +1
        return false;
    auto type = lua_rawgetp(vm, -1, tag); // +2
    lua_pop(vm, 2); // 0
    return type == LUA_TNUMBER;
}

}

}

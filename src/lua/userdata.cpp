#include "userdata.hpp"

namespace Neptools
{
namespace Lua
{

static void ClearCache(StateRef vm, void* ptr)
{
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
    NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;
    lua_pushnil(vm); // +2
    lua_rawsetp(vm, -2, ptr); // +1
    lua_pop(vm, 1); // 0
}

void RefCountedUserdataBase::Destroy(StateRef vm) noexcept
{
    auto ctrl = GetCtrl();
    ClearCache(vm, Get());
    ctrl->RemoveRef();
    this->~RefCountedUserdataBase();
}

namespace UserdataDetail
{
char TAG;

void TypeTraits::GcFun(StateRef vm)
{
    if (!lua_getmetatable(vm, 1) || // +1
        IsNoneOrNil(lua_rawgetp(vm, -1, &TAG))) // +2
        vm.TypeError(true, "neptools.object", 1);
    lua_pop(vm, 2);

    auto ub = reinterpret_cast<UserdataBase*>(lua_touserdata(vm, 1));
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

    auto ud = static_cast<UserdataBase*>(lua_touserdata(vm, idx));
    NEPTOOLS_ASSERT(ud);
    return {ud, offs};
}

UBArgs UnsafeGetBase(StateRef vm, int idx, void* tag)
{
    lua_getmetatable(vm, idx); // +1
    lua_rawgetp(vm, -1, tag); // +2
    auto offs = lua_tointeger(vm, -1);
    lua_pop(vm, 2); // 0

    auto ud = static_cast<UserdataBase*>(lua_touserdata(vm, idx));
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

template <typename Userdata>
void Push(StateRef vm, RefCounted& ctrl, void* ptr, void* tag)
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
        auto ud = lua_newuserdata(vm, sizeof(Userdata)); // +1
        auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, tag); // +2
        NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;

        new (ud) Userdata{&ctrl, ptr};
        lua_setmetatable(vm, -2); // +1

        // cache it
        lua_pushvalue(vm, -1); // +3
        lua_rawsetp(vm, -3, &ctrl); // +2
    }

    lua_remove(vm, -2); // +1 remove reftbl
    NEPTOOLS_LUA_CHECKTOP(vm, top+1);
}
template void Push<RefCountedUserdata>(StateRef, RefCounted&, void*, void*);
template void Push<SharedUserdata>(StateRef, RefCounted&, void*, void*);

}

}
}

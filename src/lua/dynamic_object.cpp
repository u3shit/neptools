#include "dynamic_object.hpp"
#include "userdata.hpp"

namespace Neptools
{
namespace Lua
{

void DynamicObject::PushLua(StateRef vm, RefCounted& ctrl)
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    // check cache
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
    NEPTOOLS_ASSERT(type == LUA_TTABLE);
    type = lua_rawgetp(vm, -1, &ctrl); // +2
    if (type != LUA_TUSERDATA) // no hit
    {
        lua_pop(vm, 1); // +1
        PushLuaObj(vm, ctrl); // +2

        // cache it
        lua_pushvalue(vm, -1); // +3
        lua_rawsetp(vm, -3, &ctrl); // +2
    }

    lua_remove(vm, -2); // +1 remove reftbl
    NEPTOOLS_LUA_CHECKTOP(vm, top+1);
}

template <> struct TypeTraits<UserdataBase>
{
    static UserdataBase& Get(StateRef vm, bool arg, int idx)
    {
        if (!TypeTraits<DynamicObject>::Is(vm, idx))
            vm.TypeError(arg, "neptools.object", idx);
        return *reinterpret_cast<UserdataBase*>(lua_touserdata(vm, idx));
    }

    static UserdataBase& UnsafeGet(StateRef vm, int idx)
    { return *reinterpret_cast<UserdataBase*>(lua_touserdata(vm, idx)); }
};

NEPTOOLS_LUAGEN(name="__gc", class="Neptools::Lua::DynamicObject")
static void Dtor(StateRef vm)
{
    if (!TypeTraits<DynamicObject>::Is(vm, 1))
        vm.TypeError(true, "neptools.object", 1);
    auto ub = reinterpret_cast<RefCountedUserdataBase*>(lua_touserdata(vm, 1));
    NEPTOOLS_ASSERT(ub);
    ub->Destroy(vm);

    lua_pushnil(vm);
    lua_setmetatable(vm, 1);
}

}
}

#include "dynamic_object.binding.hpp"

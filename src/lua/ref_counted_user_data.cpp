#include "ref_counted_user_data.hpp"

namespace Neptools::Lua
{

void RefCountedUserDataBase::Destroy(StateRef vm) noexcept
{
    ClearCache(vm);
    GetCtrl()->RemoveRef();
    this->~RefCountedUserDataBase();
}

namespace UserDataDetail
{

template <typename UserData>
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
        auto ud = lua_newuserdata(vm, sizeof(UserData)); // +1
        auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, tag); // +2
        NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;

        new (ud) UserData{&ctrl, ptr};
        lua_setmetatable(vm, -2); // +1

        // cache it
        lua_pushvalue(vm, -1); // +3
        lua_rawsetp(vm, -3, ptr); // +2
    }

    lua_remove(vm, -2); // +1 remove reftbl
    NEPTOOLS_LUA_CHECKTOP(vm, top+1);
}
template void Push<RefCountedUserData>(StateRef, RefCounted&, void*, void*);
template void Push<SharedUserData>(StateRef, RefCounted&, void*, void*);

}
}

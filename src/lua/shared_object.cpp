#include "shared_object.hpp"
#include "function_call.hpp"
#include "user_type.hpp"

namespace Neptools
{
namespace Lua
{

void SharedObject::PushLua(StateRef vm)
{
    // check cache
    lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
    lua_rawgetp(vm, -1, this); // +2
    if (!lua_isnil(vm, -1)) // hit
    {
        lua_remove(vm, -2); // +1 remove reftbl
        return;
    }
    lua_pop(vm, 1); // +1

    // create instance
    auto sptr = lua_newuserdata(vm, sizeof(std::shared_ptr<SharedObject>)); // +2
    lua_rawgetp(vm, LUA_REGISTRYINDEX, GetTypeTag()); // +3
    NEPTOOLS_ASSERT(lua_istable(vm, -1));

    new (sptr) std::shared_ptr<SharedObject>{this->shared_from_this()};
    lua_setmetatable(vm, -2); // +2

    // save
    lua_pushvalue(vm, -1); // +3
    lua_rawsetp(vm, -3, this); // +2
    lua_remove(vm, -2); // +1
}

char SharedObject::TYPE_TAG;

static void gc(StateRef vm, std::shared_ptr<SharedObject>& ptr)
{
    ptr.~shared_ptr();
    lua_pushnil(vm);
    lua_setmetatable(vm, 1);
}

template<>
void TypeRegister::DoRegister<SharedObject>(StateRef, TypeBuilder& bld)
{
    bld.Add<decltype(&gc), &gc>("__gc");
}

}
}

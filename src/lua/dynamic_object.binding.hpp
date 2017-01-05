// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.lua.dynamic_object
template<>
void TypeRegister::DoRegister<Neptools::Lua::DynamicObject>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;

    bld.Add<
        void (*)(Neptools::Lua::StateRef), &Dtor
    >("__gc");

}
static TypeRegister::StateRegister<Neptools::Lua::DynamicObject> reg_neptools_lua_dynamic_object;

}
}

const char Neptools::Lua::DynamicObject::TYPE_NAME[] = "neptools.lua.dynamic_object";


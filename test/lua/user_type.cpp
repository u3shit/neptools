#include "lua/user_type.hpp"
#include "lua/shared_object.hpp"
#include "lua/function_call.hpp"
#include <catch.hpp>

using namespace Neptools::Lua;

static int global;

struct Foo : public SharedHelper<Foo>
{
    static constexpr const char* TYPE_NAME = "foo";

    ~Foo() { global = 13; }
};

namespace Neptools { namespace Lua {
template<>
void TypeRegister::DoRegister<Foo>(StateRef, TypeBuilder& bld)
{
    bld.Inherit<SharedObject>().SharedCtor<Foo>();
}
}}

TEST_CASE("shared check memory", "[lua]")
{
    {
        State vm;
        TypeRegister::Register<Foo>(vm);

        if (luaL_dostring(vm, "local x = foo.new()"))
            FAIL(lua_tostring(vm, -1));
    }
    CHECK(global == 13);
}

// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class smart
template<>
void TypeRegister::DoRegister<Smart>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;

    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Smart, int, &Smart::x>), &::Neptools::Lua::GetMember<Smart, int, &Smart::x>
    >("get_x");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<Smart, int, &Smart::x>), &::Neptools::Lua::SetMember<Smart, int, &Smart::x>
    >("set_x");

}
static TypeRegister::StateRegister<Smart> reg_smart;

}
}


const char Smart::TYPE_NAME[] = "smart";

namespace Neptools
{
namespace Lua
{

// class foo
template<>
void TypeRegister::DoRegister<Foo>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;

    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Foo, int, &Foo::local_var>), &::Neptools::Lua::GetMember<Foo, int, &Foo::local_var>
    >("get_local_var");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<Foo, int, &Foo::local_var>), &::Neptools::Lua::SetMember<Foo, int, &Foo::local_var>
    >("set_local_var");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Foo, Smart, &Foo::smart>), &::Neptools::Lua::GetMember<Foo, Smart, &Foo::smart>
    >("get_smart");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<Foo, Smart, &Foo::smart>), &::Neptools::Lua::SetMember<Foo, Smart, &Foo::smart>
    >("set_smart");
    bld.Add<
        void (Foo::*)(int), &Foo::DoIt
    >("do_it");
    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<Foo>::Make<>), &::Neptools::Lua::TypeTraits<Foo>::Make<>
    >("new");

}
static TypeRegister::StateRegister<Foo> reg_foo;

}
}


const char Foo::TYPE_NAME[] = "foo";

namespace Neptools
{
namespace Lua
{

// class bar.baz.asdfgh
template<>
void TypeRegister::DoRegister<Bar::Baz::Asdfgh>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<Bar::Baz::Asdfgh>::Make<>), &::Neptools::Lua::TypeTraits<Bar::Baz::Asdfgh>::Make<>
    >("new");

}
static TypeRegister::StateRegister<Bar::Baz::Asdfgh> reg_bar_baz_asdfgh;

}
}


const char Bar::Baz::Asdfgh::TYPE_NAME[] = "bar.baz.asdfgh";

namespace Neptools
{
namespace Lua
{

// class baz
template<>
void TypeRegister::DoRegister<Baz>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<Baz>::Make<>), &::Neptools::Lua::TypeTraits<Baz>::Make<>
    >("new");
    bld.Add<
        void (Baz::*)(int), &Baz::SetGlobal
    >("set_global");
    bld.Add<
        int (Baz::*)(), &Baz::GetRandom
    >("get_random");

}
static TypeRegister::StateRegister<Baz> reg_baz;

}
}


const char Baz::TYPE_NAME[] = "baz";


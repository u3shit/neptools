// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class smart
template<>
void TypeRegister::DoRegister<::Smart>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Smart, int, &::Smart::x>
    >("get_x");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Smart, int, &::Smart::x>
    >("set_x");

}
static TypeRegister::StateRegister<::Smart> reg_smart;

}


const char ::Smart::TYPE_NAME[] = "smart";

namespace Neptools::Lua
{

// class foo
template<>
void TypeRegister::DoRegister<::Foo>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Foo, int, &::Foo::local_var>
    >("get_local_var");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Foo, int, &::Foo::local_var>
    >("set_local_var");
    bld.AddFunction<
        &::Neptools::Lua::GetRefCountedOwnedMember<::Foo, ::Smart, &::Foo::smart>
    >("get_smart");
    bld.AddFunction<
        static_cast<void (::Foo::*)(int)>(&::Foo::DoIt)
    >("do_it");
    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Foo>::Make<>
    >("new");

}
static TypeRegister::StateRegister<::Foo> reg_foo;

}


const char ::Foo::TYPE_NAME[] = "foo";

namespace Neptools::Lua
{

// class bar.baz.asdfgh
template<>
void TypeRegister::DoRegister<::Bar::Baz::Asdfgh>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Bar::Baz::Asdfgh>::Make<>
    >("new");

}
static TypeRegister::StateRegister<::Bar::Baz::Asdfgh> reg_bar_baz_asdfgh;

}


const char ::Bar::Baz::Asdfgh::TYPE_NAME[] = "bar.baz.asdfgh";

namespace Neptools::Lua
{

// class baz
template<>
void TypeRegister::DoRegister<::Baz>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Baz>::Make<>
    >("new");
    bld.AddFunction<
        static_cast<void (::Baz::*)(int)>(&::Baz::SetGlobal)
    >("set_global");
    bld.AddFunction<
        static_cast<int (::Baz::*)()>(&::Baz::GetRandom)
    >("get_random");

}
static TypeRegister::StateRegister<::Baz> reg_baz;

}


const char ::Baz::TYPE_NAME[] = "baz";

#endif

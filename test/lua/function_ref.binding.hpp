// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class function_ref_test
template<>
void TypeRegisterTraits<::FunctionRefTest>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        static_cast<void (::FunctionRefTest::*)(::Neptools::Lua::FunctionWrapGen<int>)>(&::FunctionRefTest::Cb<::Neptools::Lua::FunctionWrapGen<int>>)
    >("cb");
    bld.AddFunction<
        static_cast<void (::FunctionRefTest::*)(::Neptools::Lua::FunctionWrap<double(double)>)>(&::FunctionRefTest::Cb2<::Neptools::Lua::FunctionWrap<double(double)>>)
    >("cb2");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::FunctionRefTest, int, &::FunctionRefTest::x>
    >("get_x");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::FunctionRefTest, int, &::FunctionRefTest::x>
    >("set_x");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::FunctionRefTest, double, &::FunctionRefTest::y>
    >("get_y");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::FunctionRefTest, double, &::FunctionRefTest::y>
    >("set_y");

}
static TypeRegister::StateRegister<::FunctionRefTest> reg_function_ref_test;

}


const char ::FunctionRefTest::TYPE_NAME[] = "function_ref_test";

#endif

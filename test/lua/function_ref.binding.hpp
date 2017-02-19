// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class function_ref_test
template<>
void TypeRegister::DoRegister<::FunctionRefTest>(TypeBuilder& bld)
{

    bld.Add<
        void (::FunctionRefTest::*)(::Neptools::Lua::FunctionWrapGen<int>), &::FunctionRefTest::Cb<::Neptools::Lua::FunctionWrapGen<int>>
    >("cb");
    bld.Add<
        void (::FunctionRefTest::*)(::Neptools::Lua::FunctionWrap<double(double)>), &::FunctionRefTest::Cb2<::Neptools::Lua::FunctionWrap<double(double)>>
    >("cb2");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::FunctionRefTest, int, &::FunctionRefTest::x>), &::Neptools::Lua::GetMember<::FunctionRefTest, int, &::FunctionRefTest::x>
    >("get_x");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::FunctionRefTest, int, &::FunctionRefTest::x>), &::Neptools::Lua::SetMember<::FunctionRefTest, int, &::FunctionRefTest::x>
    >("set_x");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::FunctionRefTest, double, &::FunctionRefTest::y>), &::Neptools::Lua::GetMember<::FunctionRefTest, double, &::FunctionRefTest::y>
    >("get_y");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::FunctionRefTest, double, &::FunctionRefTest::y>), &::Neptools::Lua::SetMember<::FunctionRefTest, double, &::FunctionRefTest::y>
    >("set_y");

}
static TypeRegister::StateRegister<::FunctionRefTest> reg_function_ref_test;

}
}


const char ::FunctionRefTest::TYPE_NAME[] = "function_ref_test";


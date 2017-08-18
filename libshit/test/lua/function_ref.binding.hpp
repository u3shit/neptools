// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::FunctionRefTest::TYPE_NAME[] = "function_ref_test";

namespace Libshit::Lua
{

  // class function_ref_test
  template<>
  void TypeRegisterTraits<::FunctionRefTest>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      static_cast<void (::FunctionRefTest::*)(::Libshit::Lua::FunctionWrapGen<int>)>(&::FunctionRefTest::Cb<::Libshit::Lua::FunctionWrapGen<int>>)
    >("cb");
    bld.AddFunction<
      static_cast<void (::FunctionRefTest::*)(::Libshit::Lua::FunctionWrap<double(double)>)>(&::FunctionRefTest::Cb2<::Libshit::Lua::FunctionWrap<double(double)>>)
    >("cb2");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::FunctionRefTest, int, &::FunctionRefTest::x>
    >("get_x");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::FunctionRefTest, int, &::FunctionRefTest::x>
    >("set_x");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::FunctionRefTest, double, &::FunctionRefTest::y>
    >("get_y");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::FunctionRefTest, double, &::FunctionRefTest::y>
    >("set_y");

  }
  static TypeRegister::StateRegister<::FunctionRefTest> reg_function_ref_test;

}
#endif

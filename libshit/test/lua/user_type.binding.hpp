// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Smart::TYPE_NAME[] = "smart";

const char ::Foo::TYPE_NAME[] = "foo";

const char ::Bar::Baz::Asdfgh::TYPE_NAME[] = "bar.baz.asdfgh";

const char ::Baz::TYPE_NAME[] = "baz";

const char ::A::TYPE_NAME[] = "a";

const char ::B::TYPE_NAME[] = "b";

const char ::Multi::TYPE_NAME[] = "multi";

namespace Libshit::Lua
{

  // class smart
  template<>
  void TypeRegisterTraits<::Smart>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Smart, int, &::Smart::x>
    >("get_x");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Smart, int, &::Smart::x>
    >("set_x");

  }
  static TypeRegister::StateRegister<::Smart> reg_smart;

  // class foo
  template<>
  void TypeRegisterTraits<::Foo>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Foo, int, &::Foo::local_var>
    >("get_local_var");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Foo, int, &::Foo::local_var>
    >("set_local_var");
    bld.AddFunction<
      &::Libshit::Lua::GetRefCountedOwnedMember<::Foo, ::Smart, &::Foo::smart>
    >("get_smart");
    bld.AddFunction<
      static_cast<void (::Foo::*)(int)>(&::Foo::DoIt)
    >("do_it");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Foo>::Make<>
    >("new");

  }
  static TypeRegister::StateRegister<::Foo> reg_foo;

  // class bar.baz.asdfgh
  template<>
  void TypeRegisterTraits<::Bar::Baz::Asdfgh>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Bar::Baz::Asdfgh>::Make<>
    >("new");

  }
  static TypeRegister::StateRegister<::Bar::Baz::Asdfgh> reg_bar_baz_asdfgh;

  // class baz
  template<>
  void TypeRegisterTraits<::Baz>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Baz>::Make<>
    >("new");
    bld.AddFunction<
      static_cast<void (::Baz::*)(int)>(&::Baz::SetGlobal)
    >("set_global");
    bld.AddFunction<
      static_cast<int (::Baz::*)()>(&::Baz::GetRandom)
    >("get_random");

  }
  static TypeRegister::StateRegister<::Baz> reg_baz;

  // class a
  template<>
  void TypeRegisterTraits<::A>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::A, int, &::A::x>
    >("get_x");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::A, int, &::A::x>
    >("set_x");

  }
  static TypeRegister::StateRegister<::A> reg_a;

  // class b
  template<>
  void TypeRegisterTraits<::B>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::B, int, &::B::y>
    >("get_y");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::B, int, &::B::y>
    >("set_y");

  }
  static TypeRegister::StateRegister<::B> reg_b;

  // class multi
  template<>
  void TypeRegisterTraits<::Multi>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Multi, ::A, ::B>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Multi>::Make<>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Multi, SharedPtr<::B>, &::Multi::ptr>
    >("get_ptr");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Multi, SharedPtr<::B>, &::Multi::ptr>
    >("set_ptr");

  }
  static TypeRegister::StateRegister<::Multi> reg_multi;

}
#endif

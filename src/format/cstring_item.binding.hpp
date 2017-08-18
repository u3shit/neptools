// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::CStringItem::TYPE_NAME[] = "neptools.c_string_item";

namespace Libshit::Lua
{

  // class neptools.c_string_item
  template<>
  void TypeRegisterTraits<::Neptools::CStringItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::CStringItem, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::CStringItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<std::string>>,
      &::Libshit::Lua::TypeTraits<::Neptools::CStringItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<const ::Neptools::Source &>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::CStringItem & (*)(::Neptools::ItemPointer)>(::Neptools::CStringItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      static_cast<std::string (*)(std::string)>(::Neptools::CStringItem::GetLabelName),
      static_cast<std::string (::Neptools::CStringItem::*)() const>(&::Neptools::CStringItem::GetLabelName)
    >("get_label_name");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::CStringItem, std::string, &::Neptools::CStringItem::string>
    >("get_string");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::CStringItem, std::string, &::Neptools::CStringItem::string>
    >("set_string");

  }
  static TypeRegister::StateRegister<::Neptools::CStringItem> reg_neptools_c_string_item;

}
#endif

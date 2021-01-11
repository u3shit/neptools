// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::ExpansionItem::TYPE_NAME[] = "neptools.stcm.expansion_item";

namespace Libshit::Lua
{

  // class neptools.stcm.expansion_item
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::ExpansionItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stcm::ExpansionItem, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::ExpansionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint32_t>, LuaGetRef<::Neptools::LabelPtr>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::ExpansionItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::ExpansionItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::ExpansionItem, ::uint32_t, &::Neptools::Stcm::ExpansionItem::index>
    >("get_index");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::ExpansionItem, ::uint32_t, &::Neptools::Stcm::ExpansionItem::index>
    >("set_index");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::ExpansionItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::ExpansionItem::name>
    >("get_name");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::ExpansionItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::ExpansionItem::name>
    >("set_name");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::ExpansionItem> reg_neptools_stcm_expansion_item;

}
#endif

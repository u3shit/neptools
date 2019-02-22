// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::DataItem::TYPE_NAME[] = "neptools.stcm.data_item";

namespace Libshit::Lua
{

  // class neptools.stcm.data_item
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::DataItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stcm::DataItem, ::Neptools::ItemWithChildren>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::DataItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::DataItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::DataItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::type>
    >("get_type");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::type>
    >("set_type");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::offset_unit>
    >("get_offset_unit");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::offset_unit>
    >("set_offset_unit");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::field_8>
    >("get_field_8");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::field_8>
    >("set_field_8");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::DataItem> reg_neptools_stcm_data_item;

}
#endif

// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::HeaderItem::TYPE_NAME[] = "neptools.stcm.header_item";

namespace Libshit::Lua
{

  // class neptools.stcm.header_item
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::HeaderItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stcm::HeaderItem, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::HeaderItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<const ::Neptools::Stcm::HeaderItem::MsgType &>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::uint32_t>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::HeaderItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::HeaderItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::Neptools::Stcm::HeaderItem::MsgType, &::Neptools::Stcm::HeaderItem::msg>
    >("get_msg");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::Neptools::Stcm::HeaderItem::MsgType, &::Neptools::Stcm::HeaderItem::msg>
    >("set_msg");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::export_sec>
    >("get_export_sec");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::export_sec>
    >("set_export_sec");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::collection_link>
    >("get_collection_link");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::collection_link>
    >("set_collection_link");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::expansion>
    >("get_expansion");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::expansion>
    >("set_expansion");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::uint32_t, &::Neptools::Stcm::HeaderItem::field_28>
    >("get_field_28");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::uint32_t, &::Neptools::Stcm::HeaderItem::field_28>
    >("set_field_28");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::HeaderItem> reg_neptools_stcm_header_item;

}
#endif

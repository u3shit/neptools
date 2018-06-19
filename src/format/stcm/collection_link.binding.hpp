// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::CollectionLinkHeaderItem::TYPE_NAME[] = "neptools.stcm.collection_link_header_item";

const char ::Neptools::Stcm::CollectionLinkItem::TYPE_NAME[] = "neptools.stcm.collection_link_item";

const char ::Neptools::Stcm::CollectionLinkItem::LinkEntry::TYPE_NAME[] = "neptools.stcm.collection_link_item.link_entry";

namespace Libshit::Lua
{

  // class neptools.stcm.collection_link_header_item
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::CollectionLinkHeaderItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stcm::CollectionLinkHeaderItem, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkHeaderItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::CollectionLinkHeaderItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::CollectionLinkHeaderItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::CollectionLinkHeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::CollectionLinkHeaderItem::data>
    >("get_data");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stcm::CollectionLinkHeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::CollectionLinkHeaderItem::data>
    >("set_data");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::CollectionLinkHeaderItem> reg_neptools_stcm_collection_link_header_item;

  // class neptools.stcm.collection_link_item
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::CollectionLinkItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stcm::CollectionLinkItem, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>, LuaGetRef<::uint32_t>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<Libshit::AT<std::vector<::Neptools::Stcm::CollectionLinkItem::LinkEntry> >>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stcm::CollectionLinkItem, std::vector<::Neptools::Stcm::CollectionLinkItem::LinkEntry>, &::Neptools::Stcm::CollectionLinkItem::entries>
    >("get_entries");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::CollectionLinkItem> reg_neptools_stcm_collection_link_item;

  // class neptools.stcm.collection_link_item.link_entry
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::CollectionLinkItem::LinkEntry>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::CollectionLinkItem::LinkEntry, ::Neptools::LabelPtr, &::Neptools::Stcm::CollectionLinkItem::LinkEntry::name_0>
    >("get_name_0");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::CollectionLinkItem::LinkEntry, ::Neptools::LabelPtr, &::Neptools::Stcm::CollectionLinkItem::LinkEntry::name_1>
    >("get_name_1");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem::LinkEntry>::Make<LuaGetRef<::Neptools::LabelPtr>, LuaGetRef<::Neptools::LabelPtr>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::CollectionLinkItem::LinkEntry> reg_neptools_stcm_collection_link_item_link_entry;

}
#endif

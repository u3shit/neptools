// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"


const char ::Neptools::Stcm::CollectionLinkHeaderItem::TYPE_NAME[] = "neptools.stcm.collection_link_header_item";

const char ::Neptools::Stcm::CollectionLinkItem::TYPE_NAME[] = "neptools.stcm.collection_link_item";

const char ::Neptools::Stcm::CollectionLinkItem::LinkEntry::TYPE_NAME[] = "neptools.stcm.collection_link_item.link_entry";

namespace Neptools::Lua
{

// class neptools.stcm.collection_link_header_item
template<>
void TypeRegisterTraits<::Neptools::Stcm::CollectionLinkHeaderItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::CollectionLinkHeaderItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkHeaderItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::NotNull<::Neptools::LabelPtr>>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::CollectionLinkHeaderItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::CollectionLinkHeaderItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::CollectionLinkHeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::CollectionLinkHeaderItem::data>
    >("get_data");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::CollectionLinkHeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::CollectionLinkHeaderItem::data>
    >("set_data");

}
static TypeRegister::StateRegister<::Neptools::Stcm::CollectionLinkHeaderItem> reg_neptools_stcm_collection_link_header_item;

// class neptools.stcm.collection_link_item
template<>
void TypeRegisterTraits<::Neptools::Stcm::CollectionLinkItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::CollectionLinkItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>, LuaGetRef<::uint32_t>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<AT<std::vector<::Neptools::Stcm::CollectionLinkItem::LinkEntry> >>>
    >("new");
    bld.AddFunction<
        &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Stcm::CollectionLinkItem, std::vector<::Neptools::Stcm::CollectionLinkItem::LinkEntry>, &::Neptools::Stcm::CollectionLinkItem::entries>
    >("get_entries");

}
static TypeRegister::StateRegister<::Neptools::Stcm::CollectionLinkItem> reg_neptools_stcm_collection_link_item;

// class neptools.stcm.collection_link_item.link_entry
template<>
void TypeRegisterTraits<::Neptools::Stcm::CollectionLinkItem::LinkEntry>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::CollectionLinkItem::LinkEntry, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::CollectionLinkItem::LinkEntry::name_0>
    >("get_name_0");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::CollectionLinkItem::LinkEntry, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::CollectionLinkItem::LinkEntry::name_1>
    >("get_name_1");
    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::CollectionLinkItem::LinkEntry>::Make<LuaGetRef<::Neptools::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::Neptools::NotNull<::Neptools::LabelPtr>>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Stcm::CollectionLinkItem::LinkEntry> reg_neptools_stcm_collection_link_item_link_entry;

}
#endif

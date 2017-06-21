// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.stcm.header_item
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::HeaderItem>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::HeaderItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::HeaderItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<const ::Neptools::Stcm::HeaderItem::MsgType &>, LuaGetRef<::Neptools::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::Neptools::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::uint32_t>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::HeaderItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::HeaderItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::Neptools::Stcm::HeaderItem::MsgType, &::Neptools::Stcm::HeaderItem::msg>
    >("get_msg");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::Neptools::Stcm::HeaderItem::MsgType, &::Neptools::Stcm::HeaderItem::msg>
    >("set_msg");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::export_sec>
    >("get_export_sec");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::export_sec>
    >("set_export_sec");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::collection_link>
    >("get_collection_link");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::HeaderItem::collection_link>
    >("set_collection_link");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::HeaderItem, ::uint32_t, &::Neptools::Stcm::HeaderItem::field_28>
    >("get_field_28");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::HeaderItem, ::uint32_t, &::Neptools::Stcm::HeaderItem::field_28>
    >("set_field_28");

}
static TypeRegister::StateRegister<::Neptools::Stcm::HeaderItem> reg_neptools_stcm_header_item;

}


const char ::Neptools::Stcm::HeaderItem::TYPE_NAME[] = "neptools.stcm.header_item";

#endif

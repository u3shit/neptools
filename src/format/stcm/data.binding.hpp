// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.stcm.data_item
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::DataItem>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::DataItem, ::Neptools::ItemWithChildren>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::DataItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context *>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::DataItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::DataItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::type>
    >("get_type");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::type>
    >("set_type");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::offset_unit>
    >("get_offset_unit");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::offset_unit>
    >("set_offset_unit");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::field_8>
    >("get_field_8");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::DataItem, ::uint32_t, &::Neptools::Stcm::DataItem::field_8>
    >("set_field_8");

}
static TypeRegister::StateRegister<::Neptools::Stcm::DataItem> reg_neptools_stcm_data_item;

}


const char ::Neptools::Stcm::DataItem::TYPE_NAME[] = "neptools.stcm.data_item";


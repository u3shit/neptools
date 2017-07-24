// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"


const char ::Neptools::Stcm::StringDataItem::TYPE_NAME[] = "neptools.stcm.string_data_item";

namespace Neptools::Lua
{

// class neptools.stcm.string_data_item
template<>
void TypeRegisterTraits<::Neptools::Stcm::StringDataItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::StringDataItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::StringDataItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<std::string>>
    >("new");
    bld.AddFunction<
        static_cast<RefCountedPtr<::Neptools::Stcm::StringDataItem> (*)(::Neptools::Stcm::DataItem &)>(::Neptools::Stcm::StringDataItem::MaybeCreateAndReplace)
    >("maybe_create_and_replace");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::StringDataItem, std::string, &::Neptools::Stcm::StringDataItem::string>
    >("get_string");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::StringDataItem, std::string, &::Neptools::Stcm::StringDataItem::string>
    >("set_string");

}
static TypeRegister::StateRegister<::Neptools::Stcm::StringDataItem> reg_neptools_stcm_string_data_item;

}
#endif

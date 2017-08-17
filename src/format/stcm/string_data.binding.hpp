// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::StringDataItem::TYPE_NAME[] = "neptools.stcm.string_data_item";

namespace Libshit::Lua
{

// class neptools.stcm.string_data_item
template<>
void TypeRegisterTraits<::Neptools::Stcm::StringDataItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::StringDataItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::StringDataItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<std::string>>
    >("new");
    bld.AddFunction<
        static_cast<Libshit::RefCountedPtr<::Neptools::Stcm::StringDataItem> (*)(::Neptools::Stcm::DataItem &)>(::Neptools::Stcm::StringDataItem::MaybeCreateAndReplace)
    >("maybe_create_and_replace");
    bld.AddFunction<
        &::Libshit::Lua::GetMember<::Neptools::Stcm::StringDataItem, std::string, &::Neptools::Stcm::StringDataItem::string>
    >("get_string");
    bld.AddFunction<
        &::Libshit::Lua::SetMember<::Neptools::Stcm::StringDataItem, std::string, &::Neptools::Stcm::StringDataItem::string>
    >("set_string");

}
static TypeRegister::StateRegister<::Neptools::Stcm::StringDataItem> reg_neptools_stcm_string_data_item;

}
#endif

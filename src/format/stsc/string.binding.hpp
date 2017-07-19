// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"


const char ::Neptools::Stsc::StringItem::TYPE_NAME[] = "neptools.stsc.string_item";

namespace Neptools::Lua
{

// class neptools.stsc.string_item
template<>
void TypeRegisterTraits<::Neptools::Stsc::StringItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stsc::StringItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stsc::StringItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<std::string>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Stsc::StringItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<const ::Neptools::Source &>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stsc::StringItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stsc::StringItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::StringItem, std::string, &::Neptools::Stsc::StringItem::string>
    >("get_string");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::StringItem, std::string, &::Neptools::Stsc::StringItem::string>
    >("set_string");

}
static TypeRegister::StateRegister<::Neptools::Stsc::StringItem> reg_neptools_stsc_string_item;

}
#endif

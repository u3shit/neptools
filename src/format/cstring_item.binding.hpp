// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"


const char ::Neptools::CStringItem::TYPE_NAME[] = "neptools.c_string_item";

namespace Neptools::Lua
{

// class neptools.c_string_item
template<>
void TypeRegisterTraits<::Neptools::CStringItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::CStringItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::CStringItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<std::string>>,
        &::Neptools::Lua::TypeTraits<::Neptools::CStringItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<const ::Neptools::Source &>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::CStringItem & (*)(::Neptools::ItemPointer)>(::Neptools::CStringItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::CStringItem, std::string, &::Neptools::CStringItem::string>
    >("get_string");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::CStringItem, std::string, &::Neptools::CStringItem::string>
    >("set_string");

}
static TypeRegister::StateRegister<::Neptools::CStringItem> reg_neptools_c_string_item;

}
#endif

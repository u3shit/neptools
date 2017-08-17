// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>

template <>
const char ::Neptools::Int32Item::TYPE_NAME[] = "neptools.int32_item";
template <>
const char ::Neptools::FloatItem::TYPE_NAME[] = "neptools.float_item";

namespace Libshit::Lua
{

// class neptools.int32_item
template<>
void TypeRegisterTraits<::Neptools::Int32Item>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Int32Item, ::Neptools::Item>();

    bld.AddFunction<
        &::Libshit::Lua::TypeTraits<::Neptools::Int32Item>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<int>>,
        &::Libshit::Lua::TypeTraits<::Neptools::Int32Item>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Int32Item & (*)(::Neptools::ItemPointer)>(::Neptools::Int32Item::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Libshit::Lua::GetMember<::Neptools::Int32Item, int, &::Neptools::Int32Item::value>
    >("get_value");
    bld.AddFunction<
        &::Libshit::Lua::SetMember<::Neptools::Int32Item, int, &::Neptools::Int32Item::value>
    >("set_value");

}
static TypeRegister::StateRegister<::Neptools::Int32Item> reg_neptools_int32_item;

// class neptools.float_item
template<>
void TypeRegisterTraits<::Neptools::FloatItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::FloatItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Libshit::Lua::TypeTraits<::Neptools::FloatItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<float>>,
        &::Libshit::Lua::TypeTraits<::Neptools::FloatItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::FloatItem & (*)(::Neptools::ItemPointer)>(::Neptools::FloatItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Libshit::Lua::GetMember<::Neptools::FloatItem, float, &::Neptools::FloatItem::value>
    >("get_value");
    bld.AddFunction<
        &::Libshit::Lua::SetMember<::Neptools::FloatItem, float, &::Neptools::FloatItem::value>
    >("set_value");

}
static TypeRegister::StateRegister<::Neptools::FloatItem> reg_neptools_float_item;

}
#endif

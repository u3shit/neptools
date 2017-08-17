// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::GbnlItem::TYPE_NAME[] = "neptools.stcm.gbnl_item";

namespace Libshit::Lua
{

// class neptools.stcm.gbnl_item
template<>
void TypeRegisterTraits<::Neptools::Stcm::GbnlItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::GbnlItem, ::Neptools::Item, ::Neptools::Gbnl>();

    bld.AddFunction<
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::GbnlItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>,
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::GbnlItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<bool>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<Libshit::AT<::Neptools::Gbnl::Struct::TypePtr>>>,
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::GbnlItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Libshit::Lua::StateRef>, LuaGetRef<bool>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<Libshit::AT<::Neptools::Gbnl::Struct::TypePtr>>, LuaGetRef<::Libshit::Lua::RawTable>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::GbnlItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::GbnlItem::CreateAndInsert)
    >("create_and_insert");

}
static TypeRegister::StateRegister<::Neptools::Stcm::GbnlItem> reg_neptools_stcm_gbnl_item;

}
#endif

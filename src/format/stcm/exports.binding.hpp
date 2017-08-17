// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::ExportsItem::TYPE_NAME[] = "neptools.stcm.exports_item";
const char ::Libshit::Lua::TypeName<::Neptools::Stcm::ExportsItem::Type>::TYPE_NAME[] =
    "neptools.stcm.exports_item.type";

const char ::Neptools::Stcm::ExportsItem::EntryType::TYPE_NAME[] = "neptools.stcm.exports_item.entry_type";

namespace Libshit::Lua
{

// class neptools.stcm.exports_item
template<>
void TypeRegisterTraits<::Neptools::Stcm::ExportsItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::ExportsItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::ExportsItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>>,
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::ExportsItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>, LuaGetRef<::uint32_t>>,
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::ExportsItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<Libshit::AT<std::vector<::Neptools::Stcm::ExportsItem::VectorEntry> >>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::ExportsItem & (*)(::Neptools::ItemPointer, ::uint32_t)>(::Neptools::Stcm::ExportsItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stcm::ExportsItem, std::vector<::Neptools::Stcm::ExportsItem::VectorEntry>, &::Neptools::Stcm::ExportsItem::entries>
    >("get_entries");

}
static TypeRegister::StateRegister<::Neptools::Stcm::ExportsItem> reg_neptools_stcm_exports_item;

// class neptools.stcm.exports_item.type
template<>
void TypeRegisterTraits<::Neptools::Stcm::ExportsItem::Type>::Register(TypeBuilder& bld)
{

    bld.Add("CODE", ::Neptools::Stcm::ExportsItem::Type::CODE);
    bld.Add("DATA", ::Neptools::Stcm::ExportsItem::Type::DATA);

}
static TypeRegister::StateRegister<::Neptools::Stcm::ExportsItem::Type> reg_neptools_stcm_exports_item_type;

// class neptools.stcm.exports_item.entry_type
template<>
void TypeRegisterTraits<::Neptools::Stcm::ExportsItem::EntryType>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Libshit::Lua::GetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::Stcm::ExportsItem::Type, &::Neptools::Stcm::ExportsItem::EntryType::type>
    >("get_type");
    bld.AddFunction<
        &::Libshit::Lua::SetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::Stcm::ExportsItem::Type, &::Neptools::Stcm::ExportsItem::EntryType::type>
    >("set_type");
    bld.AddFunction<
        &::Libshit::Lua::GetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Libshit::FixedString<32>, &::Neptools::Stcm::ExportsItem::EntryType::name>
    >("get_name");
    bld.AddFunction<
        &::Libshit::Lua::SetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Libshit::FixedString<32>, &::Neptools::Stcm::ExportsItem::EntryType::name>
    >("set_name");
    bld.AddFunction<
        &::Libshit::Lua::GetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::ExportsItem::EntryType::lbl>
    >("get_lbl");
    bld.AddFunction<
        &::Libshit::Lua::SetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::ExportsItem::EntryType::lbl>
    >("set_lbl");
    bld.AddFunction<
        &::Libshit::Lua::TypeTraits<::Neptools::Stcm::ExportsItem::EntryType>::Make<LuaGetRef<::Neptools::Stcm::ExportsItem::Type>, LuaGetRef<const ::Libshit::FixedString<32> &>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Stcm::ExportsItem::EntryType> reg_neptools_stcm_exports_item_entry_type;

}
#endif

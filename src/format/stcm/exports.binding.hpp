// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.stcm.exports_item
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::ExportsItem>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::ExportsItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::ExportsItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context *>, LuaGetRef<::Neptools::Source>, LuaGetRef<::uint32_t>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::ExportsItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context *>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::ExportsItem & (*)(::Neptools::ItemPointer, ::uint32_t)>(::Neptools::Stcm::ExportsItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Stcm::ExportsItem, std::vector<::Neptools::NotNull<RefCountedPtr<::Neptools::Stcm::ExportsItem::EntryType> > >, &::Neptools::Stcm::ExportsItem::entries>
    >("get_entries");

}
static TypeRegister::StateRegister<::Neptools::Stcm::ExportsItem> reg_neptools_stcm_exports_item;

}


const char ::Neptools::Stcm::ExportsItem::TYPE_NAME[] = "neptools.stcm.exports_item";

namespace Neptools::Lua
{

// class neptools.stcm.exports_item.type
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::ExportsItem::Type>(TypeBuilder& bld)
{

    bld.Add("CODE", ::Neptools::Stcm::ExportsItem::Type::CODE);
    bld.Add("DATA", ::Neptools::Stcm::ExportsItem::Type::DATA);

}
static TypeRegister::StateRegister<::Neptools::Stcm::ExportsItem::Type> reg_neptools_stcm_exports_item_type;

}

const char ::Neptools::Lua::TypeName<::Neptools::Stcm::ExportsItem::Type>::TYPE_NAME[] =
    "neptools.stcm.exports_item.type";

namespace Neptools::Lua
{

// class neptools.stcm.exports_item.entry_type
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::ExportsItem::EntryType>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::Stcm::ExportsItem::Type, &::Neptools::Stcm::ExportsItem::EntryType::type>
    >("get_type");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::Stcm::ExportsItem::Type, &::Neptools::Stcm::ExportsItem::EntryType::type>
    >("set_type");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::FixedString<32>, &::Neptools::Stcm::ExportsItem::EntryType::name>
    >("get_name");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::FixedString<32>, &::Neptools::Stcm::ExportsItem::EntryType::name>
    >("set_name");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::ExportsItem::EntryType::lbl>
    >("get_lbl");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stcm::ExportsItem::EntryType, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::ExportsItem::EntryType::lbl>
    >("set_lbl");
    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::ExportsItem::EntryType>::Make<LuaGetRef<::Neptools::Stcm::ExportsItem::Type>, LuaGetRef<const ::Neptools::FixedString<32> &>, LuaGetRef<::Neptools::NotNull<::Neptools::LabelPtr>>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Stcm::ExportsItem::EntryType> reg_neptools_stcm_exports_item_entry_type;

}


const char ::Neptools::Stcm::ExportsItem::EntryType::TYPE_NAME[] = "neptools.stcm.exports_item.entry_type";


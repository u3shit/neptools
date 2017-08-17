// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stsc::HeaderItem::TYPE_NAME[] = "neptools.stsc.header_item";

namespace Neptools::Lua
{

// class neptools.stsc.header_item
template<>
void TypeRegisterTraits<::Neptools::Stsc::HeaderItem>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stsc::HeaderItem, ::Neptools::Item>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stsc::HeaderItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Stsc::HeaderItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stsc::HeaderItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::HeaderItem::entry_point>
    >("get_entry_point");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::HeaderItem::entry_point>
    >("set_entry_point");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint32_t, &::Neptools::Stsc::HeaderItem::flags>
    >("get_flags");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint32_t, &::Neptools::Stsc::HeaderItem::flags>
    >("set_flags");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, std::array<::uint8_t, 32>, &::Neptools::Stsc::HeaderItem::extra_headers_1>
    >("get_extra_headers_1");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, std::array<::uint8_t, 32>, &::Neptools::Stsc::HeaderItem::extra_headers_1>
    >("set_extra_headers_1");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_0>
    >("get_extra_headers_2_0");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_0>
    >("set_extra_headers_2_0");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_2>
    >("get_extra_headers_2_2");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_2>
    >("set_extra_headers_2_2");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_4>
    >("get_extra_headers_2_4");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_4>
    >("set_extra_headers_2_4");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_6>
    >("get_extra_headers_2_6");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_6>
    >("set_extra_headers_2_6");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_8>
    >("get_extra_headers_2_8");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_8>
    >("set_extra_headers_2_8");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_a>
    >("get_extra_headers_2_a");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_a>
    >("set_extra_headers_2_a");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_c>
    >("get_extra_headers_2_c");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_2_c>
    >("set_extra_headers_2_c");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_4>
    >("get_extra_headers_4");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::uint16_t, &::Neptools::Stsc::HeaderItem::extra_headers_4>
    >("set_extra_headers_4");

}
static TypeRegister::StateRegister<::Neptools::Stsc::HeaderItem> reg_neptools_stsc_header_item;

}
#endif

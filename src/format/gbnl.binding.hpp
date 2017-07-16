// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"


const char ::Neptools::Gbnl::TYPE_NAME[] = "neptools.gbnl";
template <>
const char ::DynStructBindgbnl::TYPE_NAME[] = "neptools.dynamic_struct_gbnl";
template <>
const char ::DynStructBindgbnl::Type::TYPE_NAME[] = "neptools.dynamic_struct_gbnl.type";
template <>
const char ::DynStructBldBindgbnl::TYPE_NAME[] = "neptools.dynamic_struct_gbnl.builder";

namespace Neptools::Lua
{

// class neptools.gbnl
template<>
void TypeRegisterTraits<::Neptools::Gbnl>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Gbnl, ::Neptools::Dumpable, ::Neptools::TxtSerializable>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<::Neptools::Source>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<bool>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::Neptools::Gbnl::Struct::TypePtr>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<::Neptools::Lua::StateRef>, LuaGetRef<bool>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::Neptools::Lua::RawTable>, LuaGetRef<::Neptools::Lua::RawTable>>
    >("new");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Gbnl, bool, &::Neptools::Gbnl::is_gstl>
    >("get_is_gstl");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Gbnl, bool, &::Neptools::Gbnl::is_gstl>
    >("set_is_gstl");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::flags>
    >("get_flags");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::flags>
    >("set_flags");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_28>
    >("get_field_28");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_28>
    >("set_field_28");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_30>
    >("get_field_30");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_30>
    >("set_field_30");
    bld.AddFunction<
        &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Gbnl, std::vector<::Neptools::Gbnl::StructPtr>, &::Neptools::Gbnl::messages>
    >("get_messages");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::Neptools::Gbnl::Struct::TypePtr, &::Neptools::Gbnl::type>
    >("get_type");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::Neptools::Gbnl::Struct::TypePtr, &::Neptools::Gbnl::type>
    >("set_type");
    bld.AddFunction<
        static_cast<void (::Neptools::Gbnl::*)()>(&::Neptools::Gbnl::RecalcSize)
    >("recalc_size");

}
static TypeRegister::StateRegister<::Neptools::Gbnl> reg_neptools_gbnl;

// class neptools.dynamic_struct_gbnl
template<>
void TypeRegisterTraits<::DynStructBindgbnl>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        static_cast<::boost::intrusive_ptr<::Neptools::DynamicStruct<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>> (*)(const ::DynStructBindgbnl::TypePtr)>(::DynStructBindgbnl::New),
        static_cast<::boost::intrusive_ptr<::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass> (*)(::Neptools::Lua::StateRef, const typename ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass::TypePtr, ::Neptools::Lua::RawTable)>(::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::New)
    >("new");
    bld.AddFunction<
        static_cast<::size_t (::DynStructBindgbnl::*)() const noexcept>(&::DynStructBindgbnl::GetSize)
    >("__len");
    bld.AddFunction<
        static_cast<::size_t (::DynStructBindgbnl::*)() const noexcept>(&::DynStructBindgbnl::GetSize)
    >("get_size");
    bld.AddFunction<
        static_cast<::DynStructBindgbnl::TypePtr (::DynStructBindgbnl::*)() const noexcept>(&::DynStructBindgbnl::GetType)
    >("get_type");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, const ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t) noexcept>(::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get),
        static_cast<void (*)(const ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::VarArg) noexcept>(::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get)
    >("get");
    bld.AddFunction<
        static_cast<void (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t, ::Neptools::Lua::Any)>(::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Set)
    >("set");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &)>(::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::ToTable)
    >("to_table");
::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Register(bld);
}
static TypeRegister::StateRegister<::DynStructBindgbnl> reg_neptools_dynamic_struct_gbnl;

// class neptools.dynamic_struct_gbnl.type
template<>
void TypeRegisterTraits<::DynStructBindgbnl::Type>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::item_count>
    >("__len");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::item_count>
    >("get_item_count");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::byte_size>
    >("get_byte_size");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, const ::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t) noexcept>(::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get),
        static_cast<void (*)(const ::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::VarArg) noexcept>(::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get)
    >("get");
    bld.AddFunction<
        static_cast<::boost::intrusive_ptr<const ::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass> (*)(::Neptools::Lua::StateRef, ::Neptools::Lua::RawTable)>(::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::New)
    >("new");

}
static TypeRegister::StateRegister<::DynStructBindgbnl::Type> reg_neptools_dynamic_struct_gbnl_type;

// class neptools.dynamic_struct_gbnl.builder
template<>
void TypeRegisterTraits<::DynStructBldBindgbnl>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::DynStructBldBindgbnl>::Make<>
    >("new");
    bld.AddFunction<
        static_cast<::DynStructBindgbnl::TypePtr (::DynStructBldBindgbnl::*)() const>(&::DynStructBldBindgbnl::Build)
    >("build");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::Raw<4>, ::size_t)>(::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Add),
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::Raw<4>)>(::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Add)
    >("add");

}
static TypeRegister::StateRegister<::DynStructBldBindgbnl> reg_neptools_dynamic_struct_gbnl_builder;

}
#endif

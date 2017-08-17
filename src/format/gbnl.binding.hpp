// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Gbnl::TYPE_NAME[] = "neptools.gbnl";
template <>
const char ::DynStructBindgbnl::TYPE_NAME[] = "neptools.dynamic_struct_gbnl";
template <>
const char ::DynStructBindgbnl::Type::TYPE_NAME[] = "neptools.dynamic_struct_gbnl.type";
template <>
const char ::DynStructBindgbnl::TypeBuilder::TYPE_NAME[] = "neptools.dynamic_struct_gbnl.builder";

namespace Neptools::Lua
{

// class neptools.gbnl
template<>
void TypeRegisterTraits<::Neptools::Gbnl>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Gbnl, ::Neptools::Dumpable, ::Neptools::TxtSerializable>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<::Neptools::Source>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<bool>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<AT<::Neptools::Gbnl::Struct::TypePtr>>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<::Neptools::Lua::StateRef>, LuaGetRef<bool>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<::uint32_t>, LuaGetRef<AT<::Neptools::Gbnl::Struct::TypePtr>>, LuaGetRef<::Neptools::Lua::RawTable>>
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
        &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Gbnl, ::Neptools::Gbnl::Messages, &::Neptools::Gbnl::messages>
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
        static_cast<::boost::intrusive_ptr<::Neptools::DynamicStruct<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>> (*)(::DynStructBindgbnl::TypePtr)>(::DynStructBindgbnl::New),
        static_cast<::boost::intrusive_ptr<::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass> (*)(::Neptools::Lua::StateRef, const typename ::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass::TypePtr, ::Neptools::Lua::RawTable)>(::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::New)
    >("new");
    bld.AddFunction<
        static_cast<::size_t (::DynStructBindgbnl::*)() const noexcept>(&::DynStructBindgbnl::GetSize)
    >("__len");
    bld.AddFunction<
        static_cast<::size_t (::DynStructBindgbnl::*)() const noexcept>(&::DynStructBindgbnl::GetSize)
    >("get_size");
    bld.AddFunction<
        static_cast<const ::DynStructBindgbnl::TypePtr & (::DynStructBindgbnl::*)() const noexcept>(&::DynStructBindgbnl::GetType)
    >("get_type");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, const ::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t) noexcept>(::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get),
        static_cast<void (*)(const ::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::VarArg) noexcept>(::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get)
    >("get");
    bld.AddFunction<
        static_cast<void (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t, ::Neptools::Lua::Any)>(::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Set)
    >("set");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &)>(::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::ToTable)
    >("to_table");
::Neptools::DynamicStructLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Register(bld);
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
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, const ::Neptools::DynamicStructTypeLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t) noexcept>(::Neptools::DynamicStructTypeLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get),
        static_cast<void (*)(const ::Neptools::DynamicStructTypeLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::VarArg) noexcept>(::Neptools::DynamicStructTypeLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get)
    >("get");
    bld.AddFunction<
        static_cast<::boost::intrusive_ptr<const ::Neptools::DynamicStructTypeLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass> (*)(::Neptools::Lua::StateRef, ::Neptools::Lua::RawTable)>(::Neptools::DynamicStructTypeLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::New)
    >("new");

}
static TypeRegister::StateRegister<::DynStructBindgbnl::Type> reg_neptools_dynamic_struct_gbnl_type;

// class neptools.dynamic_struct_gbnl.builder
template<>
void TypeRegisterTraits<::DynStructBindgbnl::TypeBuilder>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::DynStructBindgbnl::TypeBuilder>::Make<>
    >("new");
    bld.AddFunction<
        static_cast<::DynStructBindgbnl::TypePtr (::DynStructBindgbnl::TypeBuilder::*)() const>(&::DynStructBindgbnl::TypeBuilder::Build)
    >("build");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructBuilderLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::Raw<4>, ::size_t)>(::Neptools::DynamicStructBuilderLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Add),
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructBuilderLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::Raw<4>)>(::Neptools::DynamicStructBuilderLua<int8_t, int16_t, int32_t, int64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Add)
    >("add");

}
static TypeRegister::StateRegister<::DynStructBindgbnl::TypeBuilder> reg_neptools_dynamic_struct_gbnl_builder;

}
#endif

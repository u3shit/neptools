// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.gbnl
template<>
void TypeRegister::DoRegister<::Neptools::Gbnl>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Gbnl, ::Neptools::Dumpable, ::Neptools::TxtSerializable>();

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<::Neptools::Source>>), &::Neptools::Lua::TypeTraits<::Neptools::Gbnl>::Make<LuaGetRef<::Neptools::Source>>
    >("new");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Gbnl, bool, &::Neptools::Gbnl::is_gstl>), &::Neptools::Lua::GetMember<::Neptools::Gbnl, bool, &::Neptools::Gbnl::is_gstl>
    >("get_is_gstl");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Gbnl, bool, &::Neptools::Gbnl::is_gstl>), &::Neptools::Lua::SetMember<::Neptools::Gbnl, bool, &::Neptools::Gbnl::is_gstl>
    >("set_is_gstl");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::flags>), &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::flags>
    >("get_flags");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::flags>), &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::flags>
    >("set_flags");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_28>), &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_28>
    >("get_field_28");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_28>), &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_28>
    >("set_field_28");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_30>), &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_30>
    >("get_field_30");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_30>), &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::uint32_t, &::Neptools::Gbnl::field_30>
    >("set_field_30");
    bld.Add<
        decltype(&::Neptools::Lua::GetSmartOwnedMember<::Neptools::Gbnl, std::vector<::Neptools::Gbnl::StructPtr>, &::Neptools::Gbnl::messages>), &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Gbnl, std::vector<::Neptools::Gbnl::StructPtr>, &::Neptools::Gbnl::messages>
    >("get_messages");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Gbnl, ::Neptools::Gbnl::Struct::TypePtr, &::Neptools::Gbnl::type>), &::Neptools::Lua::GetMember<::Neptools::Gbnl, ::Neptools::Gbnl::Struct::TypePtr, &::Neptools::Gbnl::type>
    >("get_type");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Gbnl, ::Neptools::Gbnl::Struct::TypePtr, &::Neptools::Gbnl::type>), &::Neptools::Lua::SetMember<::Neptools::Gbnl, ::Neptools::Gbnl::Struct::TypePtr, &::Neptools::Gbnl::type>
    >("set_type");
    bld.Add<
        void (::Neptools::Gbnl::*)(), &::Neptools::Gbnl::RecalcSize
    >("recalc_size");

}
static TypeRegister::StateRegister<::Neptools::Gbnl> reg_neptools_gbnl;

}
}


const char ::Neptools::Gbnl::TYPE_NAME[] = "neptools.gbnl";

namespace Neptools
{
namespace Lua
{

// class neptools.dynamic_struct_gbnl
template<>
void TypeRegister::DoRegister<::DynStructBindgbnl>(TypeBuilder& bld)
{

    bld.Add<
        ::boost::intrusive_ptr<::Neptools::DynamicStruct<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>> (*)(const ::DynStructBindgbnl::TypePtr), &::DynStructBindgbnl::New
    >("new");
    bld.Add<
        ::size_t (::DynStructBindgbnl::*)() const, &::DynStructBindgbnl::GetSize
    >("__len");
    bld.Add<
        ::size_t (::DynStructBindgbnl::*)() const, &::DynStructBindgbnl::GetSize
    >("get_size");
    bld.Add<
        ::DynStructBindgbnl::TypePtr (::DynStructBindgbnl::*)() const, &::DynStructBindgbnl::GetType
    >("get_type");
    bld.Add<
        Overload<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, const ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t), &::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get>,
        Overload<void (*)(), &::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get>
    >("get");
    bld.Add<
        void (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t), &::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Set
    >("set");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &), &::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::ToTable
    >("to_table");
::Neptools::DynamicStructLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Register(bld);
}
static TypeRegister::StateRegister<::DynStructBindgbnl> reg_neptools_dynamic_struct_gbnl;

}
}

template <>
const char ::DynStructBindgbnl::TYPE_NAME[] = "neptools.dynamic_struct_gbnl";

namespace Neptools
{
namespace Lua
{

// class neptools.dynamic_struct_gbnl.type
template<>
void TypeRegister::DoRegister<::DynStructBindgbnl::Type>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::item_count>), &::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::item_count>
    >("__len");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::item_count>), &::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::item_count>
    >("get_item_count");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::byte_size>), &::Neptools::Lua::GetMember<::DynStructBindgbnl::Type, ::size_t, &::DynStructBindgbnl::Type::byte_size>
    >("get_byte_size");
    bld.Add<
        Overload<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, const ::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::size_t), &::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get>,
        Overload<void (*)(), &::Neptools::DynamicStructTypeLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Get>
    >("get");

}
static TypeRegister::StateRegister<::DynStructBindgbnl::Type> reg_neptools_dynamic_struct_gbnl_type;

}
}

template <>
const char ::DynStructBindgbnl::Type::TYPE_NAME[] = "neptools.dynamic_struct_gbnl.type";

namespace Neptools
{
namespace Lua
{

// class neptools.dynamic_struct_gbnl.builder
template<>
void TypeRegister::DoRegister<::DynStructBindgbnl::TypeBuilder>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::DynStructBindgbnl::TypeBuilder>::Make<>), &::Neptools::Lua::TypeTraits<::DynStructBindgbnl::TypeBuilder>::Make<>
    >("new");
    bld.Add<
        ::DynStructBindgbnl::TypePtr (::DynStructBindgbnl::TypeBuilder::*)() const, &::DynStructBindgbnl::TypeBuilder::Build
    >("build");
    bld.Add<
        Overload<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::Raw<4>, ::size_t), &::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Add>,
        Overload<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::FakeClass &, ::Neptools::Lua::Raw<4>), &::Neptools::DynamicStructBuilderLua<uint8_t, uint16_t, uint32_t, uint64_t, float, ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag, ::Neptools::Gbnl::PaddingTag>::Add>
    >("add");

}
static TypeRegister::StateRegister<::DynStructBindgbnl::TypeBuilder> reg_neptools_dynamic_struct_gbnl_builder;

}
}

template <>
const char ::DynStructBindgbnl::TypeBuilder::TYPE_NAME[] = "neptools.dynamic_struct_gbnl.builder";


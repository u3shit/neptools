// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stsc::HeaderItem::TYPE_NAME[] = "neptools.stsc.header_item";

const char ::Neptools::Stsc::HeaderItem::ExtraHeaders2::TYPE_NAME[] = "extra_headers_2";

namespace Libshit::Lua
{

  // class neptools.stsc.header_item
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::HeaderItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::HeaderItem, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::HeaderItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::HeaderItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>, LuaGetRef<std::optional<std::string_view>>, LuaGetRef<std::optional<::Neptools::Stsc::HeaderItem::ExtraHeaders2>>, LuaGetRef<std::optional<::uint16_t>>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Stsc::HeaderItem & (*)(::Neptools::ItemPointer, ::Neptools::Stsc::Flavor)>(::Neptools::Stsc::HeaderItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::HeaderItem::entry_point>
    >("get_entry_point");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::HeaderItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::HeaderItem::entry_point>
    >("set_entry_point");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem, std::optional<std::array<std::uint8_t, 32> >, &::Neptools::Stsc::HeaderItem::extra_headers_1>
    >("get_extra_headers_1");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::HeaderItem, std::optional<std::array<std::uint8_t, 32> >, &::Neptools::Stsc::HeaderItem::extra_headers_1>
    >("set_extra_headers_1");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem, std::optional<::Neptools::Stsc::HeaderItem::ExtraHeaders2>, &::Neptools::Stsc::HeaderItem::extra_headers_2>
    >("get_extra_headers_2");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::HeaderItem, std::optional<::Neptools::Stsc::HeaderItem::ExtraHeaders2>, &::Neptools::Stsc::HeaderItem::extra_headers_2>
    >("set_extra_headers_2");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem, std::optional<std::uint16_t>, &::Neptools::Stsc::HeaderItem::extra_headers_4>
    >("get_extra_headers_4");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::HeaderItem, std::optional<std::uint16_t>, &::Neptools::Stsc::HeaderItem::extra_headers_4>
    >("set_extra_headers_4");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::HeaderItem> reg_neptools_stsc_header_item;

  // class extra_headers_2
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::HeaderItem::ExtraHeaders2>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem::ExtraHeaders2, std::uint16_t, &::Neptools::Stsc::HeaderItem::ExtraHeaders2::field_0>
    >("get_field_0");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem::ExtraHeaders2, std::uint16_t, &::Neptools::Stsc::HeaderItem::ExtraHeaders2::field_2>
    >("get_field_2");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem::ExtraHeaders2, std::uint16_t, &::Neptools::Stsc::HeaderItem::ExtraHeaders2::field_4>
    >("get_field_4");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem::ExtraHeaders2, std::uint16_t, &::Neptools::Stsc::HeaderItem::ExtraHeaders2::field_6>
    >("get_field_6");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem::ExtraHeaders2, std::uint16_t, &::Neptools::Stsc::HeaderItem::ExtraHeaders2::field_8>
    >("get_field_8");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem::ExtraHeaders2, std::uint16_t, &::Neptools::Stsc::HeaderItem::ExtraHeaders2::field_a>
    >("get_field_a");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::HeaderItem::ExtraHeaders2, std::uint16_t, &::Neptools::Stsc::HeaderItem::ExtraHeaders2::field_c>
    >("get_field_c");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::HeaderItem::ExtraHeaders2>::Make<LuaGetRef<std::uint16_t>, LuaGetRef<std::uint16_t>, LuaGetRef<std::uint16_t>, LuaGetRef<std::uint16_t>, LuaGetRef<std::uint16_t>, LuaGetRef<std::uint16_t>, LuaGetRef<std::uint16_t>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::HeaderItem::ExtraHeaders2> reg_extra_headers_2;

}
#endif

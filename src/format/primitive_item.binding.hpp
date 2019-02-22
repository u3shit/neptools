// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>

template <>
const char ::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>::TYPE_NAME[] = "neptools.int32_item";
template <>
const char ::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>::TYPE_NAME[] = "neptools.float_item";

namespace Libshit::Lua
{

  // class neptools.int32_item
  template<>
  void TypeRegisterTraits<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<int>>,
      &::Libshit::Lua::TypeTraits<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'> & (*)(::Neptools::ItemPointer)>(::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>, int, &::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>::value>
    >("get_value");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>, int, &::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>::value>
    >("set_value");

  }
  static TypeRegister::StateRegister<::Neptools::PrimitiveItem<int32_t, int32_t, boost::endian::little_int32_t, 'i', 'n', 't', '3', '2'>> reg_neptools_int32_item;

  // class neptools.float_item
  template<>
  void TypeRegisterTraits<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>, ::Neptools::Item>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<float>>,
      &::Libshit::Lua::TypeTraits<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'> & (*)(::Neptools::ItemPointer)>(::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>, float, &::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>::value>
    >("get_value");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>, float, &::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>::value>
    >("set_value");

  }
  static TypeRegister::StateRegister<::Neptools::PrimitiveItem<float, int32_t, boost::endian::little_int32_t, 'f', 'l', 'o', 'a', 't'>> reg_neptools_float_item;

}
#endif

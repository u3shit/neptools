// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>

const char ::Libshit::Lua::TypeName<::Neptools::Stsc::Flavor>::TYPE_NAME[] =
  "neptools.stsc.flavor";

const char ::Neptools::Stsc::File::TYPE_NAME[] = "neptools.stsc.file";

namespace Libshit::Lua
{

  // class neptools.stsc.flavor
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::Flavor>::Register(TypeBuilder& bld)
  {

    bld.Add("NOIRE", ::Neptools::Stsc::Flavor::NOIRE);
    bld.Add("POTBB", ::Neptools::Stsc::Flavor::POTBB);

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::Flavor> reg_neptools_stsc_flavor;

  // class neptools.stsc.file
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::File>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::File, ::Neptools::Context, ::Neptools::TxtSerializable>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::File>::Make<LuaGetRef<::Neptools::Stsc::Flavor>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::File>::Make<LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::File, ::Neptools::Stsc::Flavor, &::Neptools::Stsc::File::flavor>
    >("get_flavor");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::File, ::Neptools::Stsc::Flavor, &::Neptools::Stsc::File::flavor>
    >("set_flavor");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::File> reg_neptools_stsc_file;

}
#endif

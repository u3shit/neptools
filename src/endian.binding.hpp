// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>

const char ::Libshit::Lua::TypeName<::Neptools::Endian>::TYPE_NAME[] =
  "neptools.endian";

namespace Libshit::Lua
{

  // class neptools.endian
  template<>
  void TypeRegisterTraits<::Neptools::Endian>::Register(TypeBuilder& bld)
  {

    bld.Add("BIG", ::Neptools::Endian::BIG);
    bld.Add("LITTLE", ::Neptools::Endian::LITTLE);

  }
  static TypeRegister::StateRegister<::Neptools::Endian> reg_neptools_endian;

}
#endif

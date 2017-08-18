// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::OpenFactory::TYPE_NAME[] = "neptools.open_factory";

namespace Libshit::Lua
{

  // class neptools.open_factory
  template<>
  void TypeRegisterTraits<::Neptools::OpenFactory>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      static_cast<::Libshit::NotNull<::Neptools::OpenFactory::Ret> (*)(::Neptools::Source)>(::Neptools::OpenFactory::Open),
      static_cast<::Libshit::NotNull<::Neptools::OpenFactory::Ret> (*)(const ::boost::filesystem::path &)>(::Neptools::OpenFactory::Open)
    >("open");

  }
  static TypeRegister::StateRegister<::Neptools::OpenFactory> reg_neptools_open_factory;

}
#endif

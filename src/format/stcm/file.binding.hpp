// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::File::TYPE_NAME[] = "neptools.stcm.file";

namespace Libshit::Lua
{

  // class neptools.stcm.file
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::File>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stcm::File, ::Neptools::Context, ::Neptools::TxtSerializable>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::File>::Make<>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::File>::Make<LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::GbnlItem * (::Neptools::Stcm::File::*)() const noexcept>(&::Neptools::Stcm::File::GetGbnl)
    >("get_gbnl");
    bld.AddFunction<
      static_cast<void (::Neptools::Stcm::File::*)() noexcept>(&::Neptools::Stcm::File::Gc)
    >("gc");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::File> reg_neptools_stcm_file;

}
#endif

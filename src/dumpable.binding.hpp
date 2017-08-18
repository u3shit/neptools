// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Dumpable::TYPE_NAME[] = "neptools.dumpable";

namespace Libshit::Lua
{

  // class neptools.dumpable
  template<>
  void TypeRegisterTraits<::Neptools::Dumpable>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      static_cast<void (::Neptools::Dumpable::*)()>(&::Neptools::Dumpable::Fixup)
    >("fixup");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Dumpable::*)() const>(&::Neptools::Dumpable::GetSize)
    >("get_size");
    bld.AddFunction<
      static_cast<void (::Neptools::Dumpable::*)(::Neptools::Sink &) const>(&::Neptools::Dumpable::Dump),
      static_cast<void (::Neptools::Dumpable::*)(const ::boost::filesystem::path &) const>(&::Neptools::Dumpable::Dump)
    >("dump");
    bld.AddFunction<
      static_cast<void (::Neptools::Dumpable::*)(const ::boost::filesystem::path &) const>(&::Neptools::Dumpable::Inspect),
      static_cast<std::string (::Neptools::Dumpable::*)() const>(&::Neptools::Dumpable::Inspect)
    >("inspect");

  }
  static TypeRegister::StateRegister<::Neptools::Dumpable> reg_neptools_dumpable;

}
#endif

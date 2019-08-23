#ifndef UUID_93BCB6F7_0156_4793_BAD5_76A9E9ABE263
#define UUID_93BCB6F7_0156_4793_BAD5_76A9E9ABE263
#pragma once

#include "dumpable.hpp"
#include "factory.hpp"
#include "source.hpp"

#include <libshit/lua/static_class.hpp>
#include <libshit/options.hpp>
#include <libshit/shared_ptr.hpp>

#include <functional>
#include <vector>

namespace Neptools
{

  inline Libshit::OptionGroup& GetFlavorOptions()
  {
    static Libshit::OptionGroup grp{
      Libshit::OptionParser::GetGlobal(), "Game specific options"};
    return grp;
  }

  class OpenFactory
    : public BaseFactory<Libshit::SmartPtr<Dumpable> (*)(const Source&)>,
      public Libshit::Lua::StaticClass
  {
    LIBSHIT_LUA_CLASS;
  public:
    using Ret = Libshit::SmartPtr<Dumpable>;
    LIBSHIT_NOLUA OpenFactory(BaseFactory::Fun f) : BaseFactory{f} {}

    static Libshit::NotNull<Ret> Open(Source src);
    static Libshit::NotNull<Ret> Open(const boost::filesystem::path& fname);
  };

}

#endif

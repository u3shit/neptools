#ifndef UUID_93BCB6F7_0156_4793_BAD5_76A9E9ABE263
#define UUID_93BCB6F7_0156_4793_BAD5_76A9E9ABE263
#pragma once

#include "dumpable.hpp"
#include "factory.hpp"
#include "shared_ptr.hpp"
#include "source.hpp"
#include "lua/static_class.hpp"

#include <functional>
#include <vector>

namespace Neptools
{

class OpenFactory
    : public BaseFactory<SmartPtr<Dumpable> (*)(Source)>, public Lua::StaticClass
{
    NEPTOOLS_LUA_CLASS;
public:
    using Ret = SmartPtr<Dumpable>;
    NEPTOOLS_NOLUA OpenFactory(BaseFactory::Fun f) : BaseFactory{f} {}

    static NotNull<Ret> Open(Source src);
    static NotNull<Ret> Open(const boost::filesystem::path& fname);
};

}

#endif

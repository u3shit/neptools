#ifndef UUID_93BCB6F7_0156_4793_BAD5_76A9E9ABE263
#define UUID_93BCB6F7_0156_4793_BAD5_76A9E9ABE263
#pragma once

#include "dumpable.hpp"
#include "shared_ptr.hpp"
#include "source.hpp"
#include "lua/static_class.hpp"

#include <functional>
#include <vector>

namespace Neptools
{

class OpenFactory : public Lua::StaticClass
{
    NEPTOOLS_LUA_CLASS;
public:
    using Ret = SmartPtr<Dumpable>;
    using Fun = std::function<Ret (Source)>;
    NEPTOOLS_NOLUA OpenFactory(Fun fun)
    { GetStore().push_back(std::move(fun)); }

    static NotNull<Ret> Open(Source src);
    static NotNull<Ret> Open(const boost::filesystem::path& fname);

private:
    using Store = std::vector<Fun>;
    static Store& GetStore()
    {
        static Store store;
        return store;
    }
};

}

#endif

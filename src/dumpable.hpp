#ifndef UUID_C9446864_0020_4D2F_8E96_CBC6ADCCA3BE
#define UUID_C9446864_0020_4D2F_8E96_CBC6ADCCA3BE
#pragma once

#include "meta.hpp"
#include "utils.hpp"
#include "lua/dynamic_object.hpp"
#include "lua/type_traits.hpp"

#include <boost/filesystem/path.hpp>

namespace Neptools
{

class Sink;

class Dumpable : public Lua::DynamicObject
{
    NEPTOOLS_LUA_CLASS;
public:
    Dumpable() = default;
    Dumpable(const Dumpable&) = delete;
    void operator=(const Dumpable&) = delete;
    virtual ~Dumpable() = default;

    virtual void Fixup() {};
    virtual FilePosition GetSize() const = 0;

    void Dump(Sink& os) const { return Dump_(os); }
    NEPTOOLS_NOLUA
    void Dump(Sink&& os) const { return Dump_(os); }
    void Dump(const boost::filesystem::path& path) const;

    NEPTOOLS_NOLUA
    void Inspect(std::ostream& os) const { return Inspect_(os); }
    NEPTOOLS_NOLUA
    void Inspect(std::ostream&& os) const { return Inspect_(os); }
    void Inspect(const boost::filesystem::path& path) const;
    std::string Inspect() const;

private:
    virtual void Dump_(Sink& sink) const = 0;
    virtual void Inspect_(std::ostream& os) const = 0;
};

std::ostream& operator<<(std::ostream& os, const Dumpable& dmp);

inline Lua::DynamicObject& GetDynamicObject(Dumpable& d) { return d; }

}
#endif

#ifndef UUID_E17CE799_6569_40E4_A8FE_39F088AE30AB
#define UUID_E17CE799_6569_40E4_A8FE_39F088AE30AB
#pragma once

#include "meta.hpp"
#include "lua/type_traits.hpp"
#include "lua/dynamic_object.hpp"

#include <iosfwd>
#include <string>

namespace Neptools
{

class TxtSerializable : public Lua::DynamicObject
{
    NEPTOOLS_LUA_CLASS;
public:
    NEPTOOLS_NOLUA void WriteTxt(std::ostream& os) const { WriteTxt_(os); }
    NEPTOOLS_NOLUA void WriteTxt(std::ostream&& os) const { WriteTxt_(os); }
    NEPTOOLS_NOLUA void ReadTxt(std::istream& is) { ReadTxt_(is); }
    NEPTOOLS_NOLUA void ReadTxt(std::istream&& is) { ReadTxt_(is); }

private:
    virtual void WriteTxt_(std::ostream& os) const = 0;
    virtual void ReadTxt_(std::istream& is) = 0;
};

}
#endif

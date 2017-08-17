#ifndef UUID_E17CE799_6569_40E4_A8FE_39F088AE30AB
#define UUID_E17CE799_6569_40E4_A8FE_39F088AE30AB
#pragma once

#include <libshit/meta.hpp>
#include <libshit/lua/type_traits.hpp>
#include <libshit/lua/dynamic_object.hpp>

#include <iosfwd>
#include <string>

namespace Neptools
{

class TxtSerializable : public Libshit::Lua::DynamicObject
{
    LIBSHIT_LUA_CLASS;
public:
    LIBSHIT_NOLUA void WriteTxt(std::ostream& os) const { WriteTxt_(os); }
    LIBSHIT_NOLUA void WriteTxt(std::ostream&& os) const { WriteTxt_(os); }
    LIBSHIT_NOLUA void ReadTxt(std::istream& is) { ReadTxt_(is); }
    LIBSHIT_NOLUA void ReadTxt(std::istream&& is) { ReadTxt_(is); }

private:
    virtual void WriteTxt_(std::ostream& os) const = 0;
    virtual void ReadTxt_(std::istream& is) = 0;
};

}
#endif

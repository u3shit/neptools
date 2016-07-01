#ifndef UUID_E17CE799_6569_40E4_A8FE_39F088AE30AB
#define UUID_E17CE799_6569_40E4_A8FE_39F088AE30AB
#pragma once

#include "lua/dynamic_object.hpp"
#include <iosfwd>

namespace Neptools
{

class TxtSerializable : public Lua::DynamicObject
{
public:
    static constexpr const char* TYPE_NAME = "neptools.txt_serializable";

    void WriteTxt(std::ostream& os) const { WriteTxt_(os); }
    void WriteTxt(std::ostream&& os) const { WriteTxt_(os); }
    void ReadTxt(std::istream& is) { ReadTxt_(is); }
    void ReadTxt(std::istream&& is) { ReadTxt_(is); }

private:
    virtual void WriteTxt_(std::ostream& os) const = 0;
    virtual void ReadTxt_(std::istream& is) = 0;
};

}
#endif

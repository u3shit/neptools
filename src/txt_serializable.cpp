#include "txt_serializable.hpp"
#include "lua/user_type.hpp"

namespace Neptools
{

static std::string WriteTxt(TxtSerializable& ser)
{
    std::stringstream ss;
    ser.WriteTxt(ss);
    return ss.str();
}

static void ReadTxt(TxtSerializable& ser, std::string str)
{
    std::stringstream ss{std::move(str)};
    ser.ReadTxt(ss);
}

namespace Lua
{
template<>
void TypeRegister::DoRegister<TxtSerializable>(StateRef, TypeBuilder& bld)
{
#define FT(x) decltype(&x), &x
    bld.Inherit<TxtSerializable, DynamicObject>()
        .Add<FT(WriteTxt)>("write_txt")
        .Add<FT(ReadTxt)>("read_txt")
        ;
}

static TypeRegister::StateRegister<TxtSerializable> reg;
}
}

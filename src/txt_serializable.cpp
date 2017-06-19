#include "txt_serializable.hpp"

namespace Neptools
{

NEPTOOLS_LUAGEN()
static std::string WriteTxt(TxtSerializable& ser)
{
    std::stringstream ss;
    ser.WriteTxt(ss);
    return ss.str();
}

NEPTOOLS_LUAGEN()
static void ReadTxt(TxtSerializable& ser, const std::string& str)
{
    std::stringstream ss{str};
    ser.ReadTxt(ss);
}

}

#include "txt_serializable.binding.hpp"

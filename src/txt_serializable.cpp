#include "txt_serializable.hpp"

#include <sstream>

namespace Neptools
{

LIBSHIT_LUAGEN()
static std::string WriteTxt(TxtSerializable& ser)
{
    std::stringstream ss;
    ser.WriteTxt(ss);
    return ss.str();
}

LIBSHIT_LUAGEN()
static void ReadTxt(TxtSerializable& ser, const std::string& str)
{
    std::stringstream ss{str};
    ser.ReadTxt(ss);
}

}

#include "txt_serializable.binding.hpp"

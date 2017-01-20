// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.txt_serializable
template<>
void TypeRegister::DoRegister<Neptools::TxtSerializable>(TypeBuilder& bld)
{

    bld.Add<
        std::string (*)(Neptools::TxtSerializable &), &Neptools::WriteTxt
    >("write_txt");
    bld.Add<
        void (*)(Neptools::TxtSerializable &, std::string), &Neptools::ReadTxt
    >("read_txt");

}
static TypeRegister::StateRegister<Neptools::TxtSerializable> reg_neptools_txt_serializable;

}
}


const char Neptools::TxtSerializable::TYPE_NAME[] = "neptools.txt_serializable";


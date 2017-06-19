// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.txt_serializable
template<>
void TypeRegister::DoRegister<::Neptools::TxtSerializable>(TypeBuilder& bld)
{

    bld.AddFunction<
        static_cast<std::string (*)(::Neptools::TxtSerializable &)>(&Neptools::WriteTxt)
    >("write_txt");
    bld.AddFunction<
        static_cast<void (*)(::Neptools::TxtSerializable &, const std::string &)>(&Neptools::ReadTxt)
    >("read_txt");

}
static TypeRegister::StateRegister<::Neptools::TxtSerializable> reg_neptools_txt_serializable;

}


const char ::Neptools::TxtSerializable::TYPE_NAME[] = "neptools.txt_serializable";


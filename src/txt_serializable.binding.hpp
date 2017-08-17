// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::TxtSerializable::TYPE_NAME[] = "neptools.txt_serializable";

namespace Libshit::Lua
{

// class neptools.txt_serializable
template<>
void TypeRegisterTraits<::Neptools::TxtSerializable>::Register(TypeBuilder& bld)
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
#endif

// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stsc::File::TYPE_NAME[] = "neptools.stsc.file";

namespace Neptools::Lua
{

// class neptools.stsc.file
template<>
void TypeRegisterTraits<::Neptools::Stsc::File>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stsc::File, ::Neptools::Context, ::Neptools::TxtSerializable>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stsc::File>::Make<>,
        &::Neptools::Lua::TypeTraits<::Neptools::Stsc::File>::Make<LuaGetRef<::Neptools::Source>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Stsc::File> reg_neptools_stsc_file;

}
#endif

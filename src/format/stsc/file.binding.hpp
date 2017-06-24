// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.stsc.file
template<>
void TypeRegister::DoRegister<::Neptools::Stsc::File>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stsc::File, ::Neptools::Context, ::Neptools::TxtSerializable>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stsc::File>::Make<LuaGetRef<::Neptools::Source>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Stsc::File> reg_neptools_stsc_file;

}


const char ::Neptools::Stsc::File::TYPE_NAME[] = "neptools.stsc.file";

#endif

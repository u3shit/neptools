// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.open_factory
template<>
void TypeRegisterTraits<::Neptools::OpenFactory>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        static_cast<::Neptools::NotNull<::Neptools::OpenFactory::Ret> (*)(::Neptools::Source)>(::Neptools::OpenFactory::Open),
        static_cast<::Neptools::NotNull<::Neptools::OpenFactory::Ret> (*)(const ::boost::filesystem::path &)>(::Neptools::OpenFactory::Open)
    >("open");

}
static TypeRegister::StateRegister<::Neptools::OpenFactory> reg_neptools_open_factory;

}


const char ::Neptools::OpenFactory::TYPE_NAME[] = "neptools.open_factory";

#endif

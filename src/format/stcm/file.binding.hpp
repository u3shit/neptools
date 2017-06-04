// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.stcm.file
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::File>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::File, ::Neptools::Context, ::Neptools::TxtSerializable>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::File>::Make<LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
        TableRetWrap<static_cast<std::vector<::Neptools::NotNull<SmartPtr<::Neptools::Stcm::GbnlItem> > > (::Neptools::Stcm::File::*)()>(&::Neptools::Stcm::File::FindGbnl)>::Wrap
    >("find_gbnl");

}
static TypeRegister::StateRegister<::Neptools::Stcm::File> reg_neptools_stcm_file;

}


const char ::Neptools::Stcm::File::TYPE_NAME[] = "neptools.stcm.file";


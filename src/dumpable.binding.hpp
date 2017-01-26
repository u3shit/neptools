// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.dumpable
template<>
void TypeRegister::DoRegister<::Neptools::Dumpable>(TypeBuilder& bld)
{

    bld.Add<
        void (::Neptools::Dumpable::*)(), &::Neptools::Dumpable::Fixup
    >("fixup");
    bld.Add<
        ::Neptools::FilePosition (::Neptools::Dumpable::*)() const, &::Neptools::Dumpable::GetSize
    >("get_size");
    bld.Add<
        Overload<void (::Neptools::Dumpable::*)(::Neptools::Sink &) const, &::Neptools::Dumpable::Dump>,
        Overload<void (::Neptools::Dumpable::*)(const ::boost::filesystem::path &) const, &::Neptools::Dumpable::Dump>
    >("dump");
    bld.Add<
        Overload<void (::Neptools::Dumpable::*)(const ::boost::filesystem::path &) const, &::Neptools::Dumpable::Inspect>,
        Overload<std::string (::Neptools::Dumpable::*)() const, &::Neptools::Dumpable::Inspect>
    >("inspect");

}
static TypeRegister::StateRegister<::Neptools::Dumpable> reg_neptools_dumpable;

}
}


const char ::Neptools::Dumpable::TYPE_NAME[] = "neptools.dumpable";


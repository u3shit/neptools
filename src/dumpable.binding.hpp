// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.dumpable
template<>
void TypeRegister::DoRegister<Neptools::Dumpable>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;
    bld
        .Inherit<Neptools::Dumpable, Neptools::Lua::DynamicObject>()

        .Add<
            void (Neptools::Dumpable::*)(), &Neptools::Dumpable::Fixup
        >("fixup")
        .Add<
            FilePosition (Neptools::Dumpable::*)() const, &Neptools::Dumpable::GetSize
        >("get_size")
        .Add<
            Overload<void (Neptools::Dumpable::*)(Neptools::Sink &) const, &Neptools::Dumpable::Dump>,
            Overload<void (Neptools::Dumpable::*)(const boost::filesystem::path &) const, &Neptools::Dumpable::Dump>
        >("dump")
        .Add<
            Overload<void (Neptools::Dumpable::*)(const boost::filesystem::path &) const, &Neptools::Dumpable::Inspect>,
            Overload<std::string (*)(const Neptools::Dumpable &), &InspectToString>
        >("inspect")
        ;

}
static TypeRegister::StateRegister<Neptools::Dumpable> reg_neptools_dumpable;

}
}

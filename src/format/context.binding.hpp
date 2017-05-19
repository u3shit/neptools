// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.context
template<>
void TypeRegister::DoRegister<::Neptools::Context>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Context, ::Neptools::ItemWithChildren>();

    bld.Add<
        const ::Neptools::Label & (::Neptools::Context::*)(const std::string &) const, &::Neptools::Context::GetLabel
    >("get_label");
    bld.Add<
        const ::Neptools::Label & (::Neptools::Context::*)(std::string, ::Neptools::ItemPointer), &::Neptools::Context::CreateLabel
    >("create_label");
    bld.Add<
        Overload<const ::Neptools::Label & (::Neptools::Context::*)(std::string, ::Neptools::ItemPointer), &::Neptools::Context::CreateLabelFallback>,
        Overload<const ::Neptools::Label & (::Neptools::Context::*)(std::string, ::Neptools::FilePosition), &::Neptools::Context::CreateLabelFallback>
    >("create_label_fallback");
    bld.Add<
        Overload<const ::Neptools::Label & (::Neptools::Context::*)(::Neptools::FilePosition, std::string), &::Neptools::Context::GetLabelTo>,
        Overload<const ::Neptools::Label & (::Neptools::Context::*)(::Neptools::ItemPointer), &::Neptools::Context::GetLabelTo>,
        Overload<const ::Neptools::Label & (::Neptools::Context::*)(::Neptools::FilePosition), &::Neptools::Context::GetLabelTo>
    >("get_label_to");
    bld.Add<
        ::Neptools::ItemPointer (::Neptools::Context::*)(::Neptools::FilePosition) const noexcept, &::Neptools::Context::GetPointer
    >("get_pointer");

}
static TypeRegister::StateRegister<::Neptools::Context> reg_neptools_context;

}
}


const char ::Neptools::Context::TYPE_NAME[] = "neptools.context";


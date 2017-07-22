// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"


const char ::Neptools::Context::TYPE_NAME[] = "neptools.context";

namespace Neptools::Lua
{

// class neptools.context
template<>
void TypeRegisterTraits<::Neptools::Context>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Context, ::Neptools::ItemWithChildren>();

    bld.AddFunction<
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(const std::string &) const>(&::Neptools::Context::GetLabel)
    >("get_label");
    bld.AddFunction<
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(std::string, ::Neptools::ItemPointer)>(&::Neptools::Context::CreateLabel)
    >("create_label");
    bld.AddFunction<
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(const std::string &, ::Neptools::ItemPointer)>(&::Neptools::Context::CreateLabelFallback),
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(const std::string &, ::Neptools::FilePosition)>(&::Neptools::Context::CreateLabelFallback)
    >("create_label_fallback");
    bld.AddFunction<
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(std::string, ::Neptools::ItemPointer)>(&::Neptools::Context::CreateOrSetLabel)
    >("create_or_set_label");
    bld.AddFunction<
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(std::string)>(&::Neptools::Context::GetOrCreateDummyLabel)
    >("get_or_create_dummy_label");
    bld.AddFunction<
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(::Neptools::ItemPointer)>(&::Neptools::Context::GetLabelTo),
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(::Neptools::FilePosition)>(&::Neptools::Context::GetLabelTo),
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Context::*)(::Neptools::FilePosition, std::string)>(&::Neptools::Context::GetLabelTo)
    >("get_label_to");
    bld.AddFunction<
        static_cast<::Neptools::ItemPointer (::Neptools::Context::*)(::Neptools::FilePosition) const noexcept>(&::Neptools::Context::GetPointer)
    >("get_pointer");

}
static TypeRegister::StateRegister<::Neptools::Context> reg_neptools_context;

}
#endif

// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.raw_item
template<>
void TypeRegister::DoRegister<::Neptools::RawItem>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::RawItem, ::Neptools::Item>();

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::Neptools::RawItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context *>, LuaGetRef<::Neptools::Source>, LuaGetRef<::Neptools::FilePosition>>), &::Neptools::Lua::TypeTraits<::Neptools::RawItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context *>, LuaGetRef<::Neptools::Source>, LuaGetRef<::Neptools::FilePosition>>
    >("new");
    bld.Add<
        Overload<::Neptools::RawItem::GetSourceRet (*)(::Neptools::ItemPointer, ::Neptools::FilePosition), &::Neptools::RawItem::GetSource>,
        Overload<const ::Neptools::Source & (::Neptools::RawItem::*)() const noexcept, &::Neptools::RawItem::GetSource>
    >("get_source");
    bld.Add<
        Overload<::Neptools::Item & (::Neptools::RawItem::*)(::Neptools::FilePosition, ::Neptools::NotNull<::Neptools::RefCountedPtr<::Neptools::Item> >), &::Neptools::RawItem::Split<::Neptools::Item>>,
        Overload<::Neptools::RawItem & (::Neptools::RawItem::*)(::Neptools::FilePosition, ::Neptools::FilePosition), &::Neptools::RawItem::Split>
    >("split");

}
static TypeRegister::StateRegister<::Neptools::RawItem> reg_neptools_raw_item;

}
}


const char ::Neptools::RawItem::TYPE_NAME[] = "neptools.raw_item";


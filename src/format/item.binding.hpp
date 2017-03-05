// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.item
template<>
void TypeRegister::DoRegister<::Neptools::Item>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Item, ::Neptools::Dumpable>();

    bld.Add<
        RefCountedPtr<::Neptools::Context> (::Neptools::Item::*)(), &::Neptools::Item::GetContextMaybe
    >("get_context_maybe");
    bld.Add<
        ::Neptools::NotNull<RefCountedPtr<::Neptools::Context> > (::Neptools::Item::*)(), &::Neptools::Item::GetContext
    >("get_context");
    bld.Add<
        ::Neptools::ItemWithChildren * (::Neptools::Item::*)(), &::Neptools::Item::GetParent
    >("get_parent");
    bld.Add<
        ::Neptools::FilePosition (::Neptools::Item::*)() const, &::Neptools::Item::GetPosition
    >("get_position");
    bld.Add<
        void (::Neptools::Item::*)(const ::Neptools::NotNull<RefCountedPtr<::Neptools::Item> > &), &::Neptools::Item::Replace<Check::Throw>
    >("replace");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, const ::Neptools::Item &), &Neptools::GetLabels
    >("get_labels");

}
static TypeRegister::StateRegister<::Neptools::Item> reg_neptools_item;

}
}


const char ::Neptools::Item::TYPE_NAME[] = "neptools.item";

namespace Neptools
{
namespace Lua
{

// class neptools.item_with_children
template<>
void TypeRegister::DoRegister<::Neptools::ItemWithChildren>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::ItemWithChildren, ::Neptools::Item>();

    bld.Add<
        ::Neptools::NotNull<SmartPtr<ItemList> > (*)(::Neptools::ItemWithChildren &), &Neptools::GetChildren
    >("get_children");

}
static TypeRegister::StateRegister<::Neptools::ItemWithChildren> reg_neptools_item_with_children;

}
}


const char ::Neptools::ItemWithChildren::TYPE_NAME[] = "neptools.item_with_children";


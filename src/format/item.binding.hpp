// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.label
template<>
void TypeRegisterTraits<::Neptools::Label>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Label>::Make<LuaGetRef<std::string>, LuaGetRef<::Neptools::ItemPointer>>
    >("new");
    bld.AddFunction<
        static_cast<const std::string & (::Neptools::Label::*)() const>(&::Neptools::Label::GetName)
    >("get_name");
    bld.AddFunction<
        static_cast<const ::Neptools::ItemPointer & (::Neptools::Label::*)() const>(&::Neptools::Label::GetPtr)
    >("get_ptr");

}
static TypeRegister::StateRegister<::Neptools::Label> reg_neptools_label;

}


const char ::Neptools::Label::TYPE_NAME[] = "neptools.label";

namespace Neptools::Lua
{

// class neptools.item
template<>
void TypeRegisterTraits<::Neptools::Item>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Item, ::Neptools::Dumpable>();

    bld.AddFunction<
        static_cast<RefCountedPtr<::Neptools::Context> (::Neptools::Item::*)() noexcept>(&::Neptools::Item::GetContextMaybe)
    >("get_context_maybe");
    bld.AddFunction<
        static_cast<::Neptools::NotNull<RefCountedPtr<::Neptools::Context> > (::Neptools::Item::*)()>(&::Neptools::Item::GetContext)
    >("get_context");
    bld.AddFunction<
        static_cast<::Neptools::ItemWithChildren * (::Neptools::Item::*)() noexcept>(&::Neptools::Item::GetParent)
    >("get_parent");
    bld.AddFunction<
        static_cast<::Neptools::FilePosition (::Neptools::Item::*)() const noexcept>(&::Neptools::Item::GetPosition)
    >("get_position");
    bld.AddFunction<
        static_cast<void (::Neptools::Item::*)(const ::Neptools::NotNull<RefCountedPtr<::Neptools::Item> > &)>(&::Neptools::Item::Replace<Check::Throw>)
    >("replace");
    bld.AddFunction<
        TableRetWrap<static_cast<const ::Neptools::Item::LabelsContainer & (::Neptools::Item::*)() const>(&::Neptools::Item::GetLabels)>::Wrap
    >("get_labels");

}
static TypeRegister::StateRegister<::Neptools::Item> reg_neptools_item;

}


const char ::Neptools::Item::TYPE_NAME[] = "neptools.item";

namespace Neptools::Lua
{

// class neptools.item_with_children
template<>
void TypeRegisterTraits<::Neptools::ItemWithChildren>::Register(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::ItemWithChildren, ::Neptools::Item>();

    bld.AddFunction<
        OwnedSharedPtrWrap<static_cast<::Neptools::ItemList & (::Neptools::ItemWithChildren::*)() noexcept>(&::Neptools::ItemWithChildren::GetChildren)>::Wrap
    >("get_children");

}
static TypeRegister::StateRegister<::Neptools::ItemWithChildren> reg_neptools_item_with_children;

}


const char ::Neptools::ItemWithChildren::TYPE_NAME[] = "neptools.item_with_children";

namespace Neptools::Lua
{

// class neptools.parent_list_item
template<>
void TypeRegisterTraits<::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >>::Register(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >>::Make<>
    >("new");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item &) noexcept>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::swap)
    >("swap");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::reference)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::push_back<Check::Throw>)
    >("push_back");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)()>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::pop_back<Check::Throw>)
    >("pop_back");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::reference)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::push_front<Check::Throw>)
    >("push_front");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)()>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::pop_front<Check::Throw>)
    >("pop_front");
    bld.AddFunction<
        static_cast<::item::reference (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)()>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::front<Check::Throw>)
    >("front");
    bld.AddFunction<
        static_cast<::item::reference (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)()>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::back<Check::Throw>)
    >("back");
    bld.AddFunction<
        static_cast<::item::size_type (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)() const noexcept>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::size)
    >("size");
    bld.AddFunction<
        static_cast<bool (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)() const noexcept>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::empty)
    >("empty");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::size_type) noexcept>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::shift_backwards)
    >("shift_backwards");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::size_type) noexcept>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::shift_forward)
    >("shift_forward");
    bld.AddFunction<
        static_cast<::item::iterator (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::const_iterator)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::erase<Check::Throw>),
        static_cast<::item::iterator (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::const_iterator, ::item::const_iterator)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::erase<Check::Throw>)
    >("erase");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)() noexcept>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::clear)
    >("clear");
    bld.AddFunction<
        static_cast<::item::iterator (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::const_iterator, ::item::reference)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::insert<Check::Throw>)
    >("insert");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::const_iterator, ::item &)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::splice<Check::Throw>),
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::const_iterator, ::item &, ::item::const_iterator)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::splice<Check::Throw>),
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item::const_iterator, ::item &, ::item::const_iterator, ::item::const_iterator)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::splice<Check::Throw>)
    >("splice");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::Neptools::Lua::FunctionWrapGen<bool>)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::sort<::Neptools::Lua::FunctionWrapGen<bool>>)
    >("sort");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::item &, ::Neptools::Lua::FunctionWrapGen<bool>)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::merge<::Neptools::Check::Throw, ::Neptools::Lua::FunctionWrapGen<bool>>)
    >("merge");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)() noexcept>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::reverse)
    >("reverse");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::Neptools::Lua::FunctionWrapGen<bool>)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::remove_if<::Neptools::Lua::FunctionWrapGen<bool>>)
    >("remove_if");
    bld.AddFunction<
        static_cast<void (::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::*)(::Neptools::Lua::FunctionWrapGen<bool>)>(&::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::unique<::Neptools::Lua::FunctionWrapGen<bool>>)
    >("unique");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::FakeClass &, ::Neptools::Item &)>(::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::Next)
    >("next");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::FakeClass &, ::Neptools::Item &)>(::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::Prev)
    >("prev");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::FakeClass &)>(::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::ToTable)
    >("to_table");

}
static TypeRegister::StateRegister<::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >> reg_neptools_parent_list_item;

}

template <>
const char ::Neptools::ParentList<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::TYPE_NAME[] = "neptools.parent_list_item";

#endif

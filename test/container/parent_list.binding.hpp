// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.test.parent_list_item
template<>
void TypeRegister::DoRegister<::Neptools::Test::ParentListItem>(TypeBuilder& bld)
{

    bld.Add<
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::Test::ParentListItem>::Make<LuaGetRef<int>>), &::Neptools::Lua::TypeTraits<::Neptools::Test::ParentListItem>::Make<LuaGetRef<int>>>,
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::Test::ParentListItem>::Make<>), &::Neptools::Lua::TypeTraits<::Neptools::Test::ParentListItem>::Make<>>
    >("new");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::i>), &::Neptools::Lua::GetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::i>
    >("get_i");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::i>), &::Neptools::Lua::SetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::i>
    >("set_i");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::data>), &::Neptools::Lua::GetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::data>
    >("get_data");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::data>), &::Neptools::Lua::SetMember<::Neptools::Test::ParentListItem, int, &::Neptools::Test::ParentListItem::data>
    >("set_data");

}
static TypeRegister::StateRegister<::Neptools::Test::ParentListItem> reg_neptools_test_parent_list_item;

}
}


const char ::Neptools::Test::ParentListItem::TYPE_NAME[] = "neptools.test.parent_list_item";

namespace Neptools
{
namespace Lua
{

// class neptools.parent_list_parent_list_item
template<>
void TypeRegister::DoRegister<::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >>::Make<>), &::Neptools::Lua::TypeTraits<::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >>::Make<>
    >("new");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> > &), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::swap
    >("swap");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::reference), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::push_back<Check::Throw>
    >("push_back");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::pop_back<Check::Throw>
    >("pop_back");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::reference), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::push_front<Check::Throw>
    >("push_front");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::pop_front<Check::Throw>
    >("pop_front");
    bld.Add<
        ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::reference (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::front<Check::Throw>
    >("front");
    bld.Add<
        ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::reference (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::back<Check::Throw>
    >("back");
    bld.Add<
        ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::size_type (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)() const, &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::size
    >("size");
    bld.Add<
        bool (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)() const, &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::empty
    >("empty");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::size_type), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::shift_backwards
    >("shift_backwards");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::size_type), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::shift_forward
    >("shift_forward");
    bld.Add<
        Overload<::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::iterator (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::erase<Check::Throw>>,
        Overload<::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::iterator (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::erase<Check::Throw>>
    >("erase");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::clear
    >("clear");
    bld.Add<
        ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::iterator (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::reference), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::insert<Check::Throw>
    >("insert");
    bld.Add<
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> > &, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::splice<Check::Throw>>,
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> > &, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::splice<Check::Throw>>,
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_iterator, ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> > &), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::splice<Check::Throw>>
    >("splice");
    bld.Add<
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::Lua::FunctionWrapGen<bool>), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::sort<::Neptools::Lua::FunctionWrapGen<bool>>>,
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::sort>
    >("sort");
    bld.Add<
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> > &, ::Neptools::Lua::FunctionWrapGen<bool>), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::merge<::Neptools::Check::Throw, ::Neptools::Lua::FunctionWrapGen<bool>>>,
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> > &), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::merge<Check::Throw>>
    >("merge");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::reverse
    >("reverse");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::const_reference), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::remove
    >("remove");
    bld.Add<
        void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::Lua::FunctionWrapGen<bool>), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::remove_if<::Neptools::Lua::FunctionWrapGen<bool>>
    >("remove_if");
    bld.Add<
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(::Neptools::Lua::FunctionWrapGen<bool>), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::unique<::Neptools::Lua::FunctionWrapGen<bool>>>,
        Overload<void (::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::*)(), &::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::unique>
    >("unique");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits>::FakeClass &, ::Neptools::Test::ParentListItem &), &::Neptools::ParentListLua<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits>::Next
    >("next");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits>::FakeClass &, ::Neptools::Test::ParentListItem &), &::Neptools::ParentListLua<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits>::Prev
    >("prev");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits>::FakeClass &), &::Neptools::ParentListLua<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits>::ToTable
    >("to_table");

}
static TypeRegister::StateRegister<::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >> reg_neptools_parent_list_parent_list_item;

}
}

template <>
const char ::Neptools::ParentList<::Neptools::Test::ParentListItem, ::Neptools::Test::ParentListItemTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Test::ParentListItem, ::Neptools::DefaultTag> >::TYPE_NAME[] = "neptools.parent_list_parent_list_item";


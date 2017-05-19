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
void TypeRegister::DoRegister<::parent_list_item>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::parent_list_item>::Make<>), &::Neptools::Lua::TypeTraits<::parent_list_item>::Make<>
    >("new");
    bld.Add<
        void (::parent_list_item::*)(::parent_list_item &) noexcept, &::parent_list_item::swap
    >("swap");
    bld.Add<
        void (::parent_list_item::*)(::parent_list_item::reference), &::parent_list_item::push_back<Check::Throw>
    >("push_back");
    bld.Add<
        void (::parent_list_item::*)(), &::parent_list_item::pop_back<Check::Throw>
    >("pop_back");
    bld.Add<
        void (::parent_list_item::*)(::parent_list_item::reference), &::parent_list_item::push_front<Check::Throw>
    >("push_front");
    bld.Add<
        void (::parent_list_item::*)(), &::parent_list_item::pop_front<Check::Throw>
    >("pop_front");
    bld.Add<
        ::parent_list_item::reference (::parent_list_item::*)(), &::parent_list_item::front<Check::Throw>
    >("front");
    bld.Add<
        ::parent_list_item::reference (::parent_list_item::*)(), &::parent_list_item::back<Check::Throw>
    >("back");
    bld.Add<
        ::parent_list_item::size_type (::parent_list_item::*)() const noexcept, &::parent_list_item::size
    >("size");
    bld.Add<
        bool (::parent_list_item::*)() const noexcept, &::parent_list_item::empty
    >("empty");
    bld.Add<
        void (::parent_list_item::*)(::parent_list_item::size_type) noexcept, &::parent_list_item::shift_backwards
    >("shift_backwards");
    bld.Add<
        void (::parent_list_item::*)(::parent_list_item::size_type) noexcept, &::parent_list_item::shift_forward
    >("shift_forward");
    bld.Add<
        Overload<::parent_list_item::iterator (::parent_list_item::*)(::parent_list_item::const_iterator, ::parent_list_item::const_iterator), &::parent_list_item::erase<Check::Throw>>,
        Overload<::parent_list_item::iterator (::parent_list_item::*)(::parent_list_item::const_iterator), &::parent_list_item::erase<Check::Throw>>
    >("erase");
    bld.Add<
        void (::parent_list_item::*)() noexcept, &::parent_list_item::clear
    >("clear");
    bld.Add<
        ::parent_list_item::iterator (::parent_list_item::*)(::parent_list_item::const_iterator, ::parent_list_item::reference), &::parent_list_item::insert<Check::Throw>
    >("insert");
    bld.Add<
        Overload<void (::parent_list_item::*)(::parent_list_item::const_iterator, ::parent_list_item &, ::parent_list_item::const_iterator, ::parent_list_item::const_iterator), &::parent_list_item::splice<Check::Throw>>,
        Overload<void (::parent_list_item::*)(::parent_list_item::const_iterator, ::parent_list_item &, ::parent_list_item::const_iterator), &::parent_list_item::splice<Check::Throw>>,
        Overload<void (::parent_list_item::*)(::parent_list_item::const_iterator, ::parent_list_item &), &::parent_list_item::splice<Check::Throw>>
    >("splice");
    bld.Add<
        Overload<void (::parent_list_item::*)(::Neptools::Lua::FunctionWrapGen<bool>), &::parent_list_item::sort<::Neptools::Lua::FunctionWrapGen<bool>>>,
        Overload<void (::parent_list_item::*)(), &::parent_list_item::sort>
    >("sort");
    bld.Add<
        Overload<void (::parent_list_item::*)(::parent_list_item &, ::Neptools::Lua::FunctionWrapGen<bool>), &::parent_list_item::merge<::Neptools::Check::Throw, ::Neptools::Lua::FunctionWrapGen<bool>>>,
        Overload<void (::parent_list_item::*)(::parent_list_item &), &::parent_list_item::merge<Check::Throw>>
    >("merge");
    bld.Add<
        void (::parent_list_item::*)() noexcept, &::parent_list_item::reverse
    >("reverse");
    bld.Add<
        void (::parent_list_item::*)(::parent_list_item::const_reference), &::parent_list_item::remove
    >("remove");
    bld.Add<
        void (::parent_list_item::*)(::Neptools::Lua::FunctionWrapGen<bool>), &::parent_list_item::remove_if<::Neptools::Lua::FunctionWrapGen<bool>>
    >("remove_if");
    bld.Add<
        Overload<void (::parent_list_item::*)(::Neptools::Lua::FunctionWrapGen<bool>), &::parent_list_item::unique<::Neptools::Lua::FunctionWrapGen<bool>>>,
        Overload<void (::parent_list_item::*)(), &::parent_list_item::unique>
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
static TypeRegister::StateRegister<::parent_list_item> reg_neptools_parent_list_parent_list_item;

}
}

template <>
const char ::parent_list_item::TYPE_NAME[] = "neptools.parent_list_parent_list_item";


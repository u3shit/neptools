// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.test.om_item_test
template<>
void TypeRegister::DoRegister<::Neptools::Test::OMItemTest>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::Neptools::Test::OMItemTest>::Make<LuaGetRef<std::string>, LuaGetRef<int>>), &::Neptools::Lua::TypeTraits<::Neptools::Test::OMItemTest>::Make<LuaGetRef<std::string>, LuaGetRef<int>>
    >("new");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Test::OMItemTest, std::string, &::Neptools::Test::OMItemTest::k>), &::Neptools::Lua::GetMember<::Neptools::Test::OMItemTest, std::string, &::Neptools::Test::OMItemTest::k>
    >("get_k");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Test::OMItemTest, std::string, &::Neptools::Test::OMItemTest::k>), &::Neptools::Lua::SetMember<::Neptools::Test::OMItemTest, std::string, &::Neptools::Test::OMItemTest::k>
    >("set_k");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Test::OMItemTest, int, &::Neptools::Test::OMItemTest::v>), &::Neptools::Lua::GetMember<::Neptools::Test::OMItemTest, int, &::Neptools::Test::OMItemTest::v>
    >("get_v");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Test::OMItemTest, int, &::Neptools::Test::OMItemTest::v>), &::Neptools::Lua::SetMember<::Neptools::Test::OMItemTest, int, &::Neptools::Test::OMItemTest::v>
    >("set_v");

}
static TypeRegister::StateRegister<::Neptools::Test::OMItemTest> reg_neptools_test_om_item_test;

}
}


const char ::Neptools::Test::OMItemTest::TYPE_NAME[] = "neptools.test.om_item_test";

namespace Neptools
{
namespace Lua
{

// class neptools.ordered_map_om_item_test
template<>
void TypeRegister::DoRegister<::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >>::Make<>), &::Neptools::Lua::TypeTraits<::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >>::Make<>
    >("new");
    bld.Add<
        ::Neptools::Test::OMItemTest & (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(::size_t), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::at
    >("at");
    bld.Add<
        ::Neptools::Test::OMItemTest & (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::front<Check::Throw>
    >("front");
    bld.Add<
        ::Neptools::Test::OMItemTest & (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::back<Check::Throw>
    >("back");
    bld.Add<
        bool (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::empty
    >("empty");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::size
    >("__len");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::size
    >("size");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::max_size
    >("max_size");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(::size_t), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::reserve
    >("reserve");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::capacity
    >("capacity");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::shrink_to_fit
    >("shrink_to_fit");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::clear
    >("clear");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::pop_back<Check::Throw>
    >("pop_back");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &), &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::swap
    >("swap");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(const ::Neptools::Test::OMItemTest &) const, &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::index_of<Check::Throw>
    >("index_of");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::*)(const ::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::key_type &) const, &::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::count
    >("count");
    bld.Add<
        Overload<SmartPtr<::Neptools::Test::OMItemTest> (*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get>,
        Overload<SmartPtr<::Neptools::Test::OMItemTest> (*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, const typename ::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::key_type &), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get>,
        Overload<void (*)(), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get>
    >("get");
    bld.Add<
        std::tuple<bool, size_t> (*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, ::size_t, ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > &&), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::insert
    >("insert");
    bld.Add<
        Overload<::size_t (*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, ::size_t, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::erase>,
        Overload<::size_t (*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::erase>
    >("erase");
    bld.Add<
        ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > (*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::remove
    >("remove");
    bld.Add<
        std::tuple<bool, size_t> (*)(::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > &&), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::push_back
    >("push_back");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &, const typename ::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::key_type &), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::find
    >("find");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > > &), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::to_table
    >("to_table");
  luaL_getmetatable(bld, "neptools_ipairs");  bld.SetField("__ipairs");
}
static TypeRegister::StateRegister<::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >> reg_neptools_ordered_map_om_item_test;

}
}

template <>
const char ::Neptools::OrderedMap<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits, ::std::less<::std::basic_string<char> > >::TYPE_NAME[] = "neptools.ordered_map_om_item_test";


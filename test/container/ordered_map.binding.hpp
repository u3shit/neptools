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
void TypeRegister::DoRegister<::om_item_test>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::om_item_test>::Make<>), &::Neptools::Lua::TypeTraits<::om_item_test>::Make<>
    >("new");
    bld.Add<
        ::Neptools::Test::OMItemTest & (::om_item_test::*)(::size_t), &::om_item_test::at
    >("at");
    bld.Add<
        ::Neptools::Test::OMItemTest & (::om_item_test::*)(), &::om_item_test::front<Check::Throw>
    >("front");
    bld.Add<
        ::Neptools::Test::OMItemTest & (::om_item_test::*)(), &::om_item_test::back<Check::Throw>
    >("back");
    bld.Add<
        bool (::om_item_test::*)() const noexcept, &::om_item_test::empty
    >("empty");
    bld.Add<
        ::size_t (::om_item_test::*)() const noexcept, &::om_item_test::size
    >("__len");
    bld.Add<
        ::size_t (::om_item_test::*)() const noexcept, &::om_item_test::size
    >("size");
    bld.Add<
        ::size_t (::om_item_test::*)() const noexcept, &::om_item_test::max_size
    >("max_size");
    bld.Add<
        void (::om_item_test::*)(::size_t), &::om_item_test::reserve
    >("reserve");
    bld.Add<
        ::size_t (::om_item_test::*)() const noexcept, &::om_item_test::capacity
    >("capacity");
    bld.Add<
        void (::om_item_test::*)(), &::om_item_test::shrink_to_fit
    >("shrink_to_fit");
    bld.Add<
        void (::om_item_test::*)() noexcept, &::om_item_test::clear
    >("clear");
    bld.Add<
        void (::om_item_test::*)() noexcept, &::om_item_test::pop_back<Check::Throw>
    >("pop_back");
    bld.Add<
        void (::om_item_test::*)(::om_item_test &), &::om_item_test::swap
    >("swap");
    bld.Add<
        ::size_t (::om_item_test::*)(const ::Neptools::Test::OMItemTest &) const, &::om_item_test::index_of<Check::Throw>
    >("index_of");
    bld.Add<
        ::size_t (::om_item_test::*)(const ::om_item_test::key_type &) const, &::om_item_test::count
    >("count");
    bld.Add<
        Overload<SmartPtr<::Neptools::Test::OMItemTest> (*)(::om_item_test &, ::size_t) noexcept, &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get>,
        Overload<SmartPtr<::Neptools::Test::OMItemTest> (*)(::om_item_test &, const typename ::om_item_test::key_type &), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get>,
        Overload<void (*)() noexcept, &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get>
    >("get");
    bld.Add<
        std::tuple<bool, ::size_t> (*)(::om_item_test &, ::size_t, ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > &&), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::insert
    >("insert");
    bld.Add<
        Overload<::size_t (*)(::om_item_test &, ::size_t, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::erase>,
        Overload<::size_t (*)(::om_item_test &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::erase>
    >("erase");
    bld.Add<
        ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > (*)(::om_item_test &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::remove
    >("remove");
    bld.Add<
        std::tuple<bool, ::size_t> (*)(::om_item_test &, ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > &&), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::push_back
    >("push_back");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::om_item_test &, const typename ::om_item_test::key_type &), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::find
    >("find");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::om_item_test &), &::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::to_table
    >("to_table");
  luaL_getmetatable(bld, "neptools_ipairs");  bld.SetField("__ipairs");
}
static TypeRegister::StateRegister<::om_item_test> reg_neptools_ordered_map_om_item_test;

}
}

template <>
const char ::om_item_test::TYPE_NAME[] = "neptools.ordered_map_om_item_test";


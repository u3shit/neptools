// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.test.om_item_test
template<>
void TypeRegister::DoRegister<::Neptools::Test::OMItemTest>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Test::OMItemTest>::Make<LuaGetRef<std::string>, LuaGetRef<int>>
    >("new");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Test::OMItemTest, std::string, &::Neptools::Test::OMItemTest::k>
    >("get_k");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Test::OMItemTest, std::string, &::Neptools::Test::OMItemTest::k>
    >("set_k");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Test::OMItemTest, int, &::Neptools::Test::OMItemTest::v>
    >("get_v");
    bld.AddFunction<
        &::Neptools::Lua::SetMember<::Neptools::Test::OMItemTest, int, &::Neptools::Test::OMItemTest::v>
    >("set_v");

}
static TypeRegister::StateRegister<::Neptools::Test::OMItemTest> reg_neptools_test_om_item_test;

}


const char ::Neptools::Test::OMItemTest::TYPE_NAME[] = "neptools.test.om_item_test";

namespace Neptools::Lua
{

// class neptools.ordered_map_om_item_test
template<>
void TypeRegister::DoRegister<::om_item_test>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::om_item_test>::Make<>
    >("new");
    bld.AddFunction<
        static_cast<::Neptools::Test::OMItemTest & (::om_item_test::*)(::size_t)>(&::om_item_test::at)
    >("at");
    bld.AddFunction<
        static_cast<::Neptools::Test::OMItemTest & (::om_item_test::*)()>(&::om_item_test::front<Check::Throw>)
    >("front");
    bld.AddFunction<
        static_cast<::Neptools::Test::OMItemTest & (::om_item_test::*)()>(&::om_item_test::back<Check::Throw>)
    >("back");
    bld.AddFunction<
        static_cast<bool (::om_item_test::*)() const noexcept>(&::om_item_test::empty)
    >("empty");
    bld.AddFunction<
        static_cast<::size_t (::om_item_test::*)() const noexcept>(&::om_item_test::size)
    >("__len");
    bld.AddFunction<
        static_cast<::size_t (::om_item_test::*)() const noexcept>(&::om_item_test::size)
    >("size");
    bld.AddFunction<
        static_cast<::size_t (::om_item_test::*)() const noexcept>(&::om_item_test::max_size)
    >("max_size");
    bld.AddFunction<
        static_cast<void (::om_item_test::*)(::size_t)>(&::om_item_test::reserve)
    >("reserve");
    bld.AddFunction<
        static_cast<::size_t (::om_item_test::*)() const noexcept>(&::om_item_test::capacity)
    >("capacity");
    bld.AddFunction<
        static_cast<void (::om_item_test::*)()>(&::om_item_test::shrink_to_fit)
    >("shrink_to_fit");
    bld.AddFunction<
        static_cast<void (::om_item_test::*)() noexcept>(&::om_item_test::clear)
    >("clear");
    bld.AddFunction<
        static_cast<void (::om_item_test::*)() noexcept>(&::om_item_test::pop_back<Check::Throw>)
    >("pop_back");
    bld.AddFunction<
        static_cast<void (::om_item_test::*)(::om_item_test &)>(&::om_item_test::swap)
    >("swap");
    bld.AddFunction<
        static_cast<::size_t (::om_item_test::*)(const ::Neptools::Test::OMItemTest &) const>(&::om_item_test::index_of<Check::Throw>)
    >("index_of");
    bld.AddFunction<
        static_cast<::size_t (::om_item_test::*)(const ::om_item_test::key_type &) const>(&::om_item_test::count)
    >("count");
    bld.AddFunction<
        static_cast<SmartPtr<::Neptools::Test::OMItemTest> (*)(::om_item_test &, ::size_t) noexcept>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get),
        static_cast<SmartPtr<::Neptools::Test::OMItemTest> (*)(::om_item_test &, const typename ::om_item_test::key_type &)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get),
        static_cast<void (*)() noexcept>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::get)
    >("get");
    bld.AddFunction<
        static_cast<std::tuple<bool, ::size_t> (*)(::om_item_test &, ::size_t, ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > &&)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::insert)
    >("insert");
    bld.AddFunction<
        static_cast<::size_t (*)(::om_item_test &, ::size_t, ::size_t)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::erase),
        static_cast<::size_t (*)(::om_item_test &, ::size_t)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::erase)
    >("erase");
    bld.AddFunction<
        static_cast<::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > (*)(::om_item_test &, ::size_t)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::remove)
    >("remove");
    bld.AddFunction<
        static_cast<std::tuple<bool, ::size_t> (*)(::om_item_test &, ::Neptools::NotNull<SmartPtr<::Neptools::Test::OMItemTest> > &&)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::push_back)
    >("push_back");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::om_item_test &, const typename ::om_item_test::key_type &)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::find)
    >("find");
    bld.AddFunction<
        static_cast<::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::om_item_test &)>(::Neptools::OrderedMapLua<::Neptools::Test::OMItemTest, ::Neptools::Test::OMItemTestTraits>::to_table)
    >("to_table");
  luaL_getmetatable(bld, "neptools_ipairs");  bld.SetField("__ipairs");
}
static TypeRegister::StateRegister<::om_item_test> reg_neptools_ordered_map_om_item_test;

}

template <>
const char ::om_item_test::TYPE_NAME[] = "neptools.ordered_map_om_item_test";

#endif

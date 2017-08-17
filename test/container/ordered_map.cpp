#include "container/ordered_map.hpp"
#include <libshit/lua/base.hpp>
#include <catch.hpp>

using namespace Neptools;
using namespace Neptools::Lua;

namespace Neptools::Test
{
struct OMItemTest final : public OrderedMapItem, public Lua::DynamicObject
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    OMItemTest(std::string k, int v) : k{std::move(k)}, v{v} { ++count; }
    ~OMItemTest() { --count; }
    OMItemTest(const OMItemTest&) = delete;
    void operator=(const OMItemTest&) = delete;

    std::string k;
    int v;
    static size_t count;

    bool operator==(const OMItemTest& o) const noexcept
    { return k == o.k && v == o.v; }
};
std::ostream& operator<<(std::ostream& os, const OMItemTest& i)
{ return os << "OMItemTest{" << i.k << ", " << i.v << '}'; }
size_t OMItemTest::count;

struct OMItemTestTraits
{
    using type = std::string;
    const std::string& operator()(const OMItemTest& x) { return x.k; }
};
}

using X = Neptools::Test::OMItemTest;
using OM = OrderedMap<Neptools::Test::OMItemTest, Neptools::Test::OMItemTestTraits>;

TEST_CASE("OrderedMap::basic", "[ordered_map]")
{
    X::count = 0;
    {
        OM om;
        CHECK(om.empty());
        CHECK(om.size() == 0);
        om.emplace_back("foo",2);
        om.emplace_back("bar",7);
        CHECK(!om.empty());
        CHECK(om.size() == 2);

        SECTION("at")
        {
            CHECK(om.at(0) == X("foo", 2));
            CHECK(om.at(1) == X("bar", 7));
            CHECK_THROWS(om.at(2));
        }

        SECTION("operator[]")
        {
            CHECK(om[0] == X("foo", 2));
            CHECK(om[1] == X("bar", 7));
        }

        SECTION("front/back")
        {
            CHECK(om.front() == X("foo", 2));
            CHECK(om.back() == X("bar", 7));
        }

        SECTION("iterator")
        {
            auto it = om.begin();
            REQUIRE(it != om.end());
            CHECK(it->k == "foo");
            ++it;
            REQUIRE(it != om.end());
            CHECK(*it == X("bar", 7));
            ++it;
            CHECK(it == om.end());
        }

        SECTION("reserve")
        {
            CHECK(om.capacity() >= 2);
            om.reserve(10);
            REQUIRE(om.size() == 2);
            CHECK(om.capacity() >= 10);

            CHECK(om[1] == X("bar", 7));
        }

        SECTION("clear")
        {
            om.clear();
            CHECK(om.empty());
        }

        SECTION("insert")
        {
            om.insert(om.begin()+1, MakeSmart<X>("def", 9));
            REQUIRE(om.size() == 3);
            CHECK(om[0] == X("foo", 2));
            CHECK(om[1] == X("def", 9));
            CHECK(om[2] == X("bar", 7));
        }

        SECTION("insert existing")
        {
            om.insert(om.begin(), MakeSmart<X>("bar", -1));
            REQUIRE(om.size() == 2);
            CHECK(om[0] == X("foo", 2));
            CHECK(om[1] == X("bar", 7));
        }

        SECTION("emplace")
        {
            om.emplace(om.begin(), "aa", 5);
            REQUIRE(om.size() == 3);
            CHECK(om[0] == X("aa", 5));
            CHECK(om[1] == X("foo", 2));
            CHECK(om[2] == X("bar", 7));
        }

        SECTION("emplace existing")
        {
            om.emplace(om.begin(), "foo", -1);
            REQUIRE(om.size() == 2);
            CHECK(om[0] == X("foo", 2));
            CHECK(om[1] == X("bar", 7));
        }

        SECTION("erase one")
        {
            om.erase(om.begin());
            REQUIRE(om.size() == 1);
            CHECK(om[0] == X("bar", 7));
        }

        SECTION("erase range")
        {
            om.erase(om.begin(), om.begin()+1);
            REQUIRE(om.size() == 1);
            CHECK(om[0] == X("bar", 7));
        }

        SECTION("erase everything")
        {
            om.erase(om.begin(), om.end());
            CHECK(om.empty());
        }

        SECTION("push_back")
        {
            om.push_back(MakeSmart<X>("zed",3));
            REQUIRE(om.size() == 3);
            CHECK(om[0] == X("foo", 2));
            CHECK(om[1] == X("bar", 7));
            CHECK(om[2] == X("zed", 3));
        }

        SECTION("push_back existing")
        {
            om.push_back(MakeSmart<X>("bar",77));
            REQUIRE(om.size() == 2);
            CHECK(om[0] == X("foo", 2));
            CHECK(om[1] == X("bar", 7));
        }

        SECTION("pop_back")
        {
            om.pop_back();
            REQUIRE(om.size() == 1);
            CHECK(om[0] == X("foo", 2));
            om.pop_back();
            REQUIRE(om.empty());
        }

        SECTION("nth")
        {
            CHECK(om.nth(0) == om.begin());
            CHECK(om.nth(0)->k == "foo");
            CHECK(*om.nth(1) == X("bar", 7));
            CHECK(om.nth(2) == om.end());
            CHECK(om.index_of(om.nth(1)) == 1);
        }

        SECTION("map find")
        {
            CHECK(om.count("foo") == 1);
            CHECK(om.count("baz") == 0);
            CHECK(*om.find("foo") == X("foo", 2));
            CHECK(om.find("baz") == om.end());
        }

        SECTION("iterator_to")
        {
            X& x = om[1];
            CHECK(om.iterator_to(x) == om.nth(1));
        }

        SECTION("key_change")
        {
            om[0].k = "abc";
            CHECK(om.count("abc") == 0); // wrong rb-tree
            om.key_change(om.begin());
            CHECK(om.count("abc") == 1); // fixed
        }
    }
    CHECK(X::count == 0);
}

#ifndef NEPTOOLS_WITHOUT_LUA
TEST_CASE("OrderedMap::lua", "[ordered_map]")
{
    State vm;
    auto om = MakeSmart<OM>();
    vm.Push(om);
    lua_setglobal(vm, "om");
    vm.DoString("it = neptools.test.om_item_test");

    SECTION("lua push_back")
    {
        vm.DoString("om:push_back(it('xy',2))");
        CHECK(om->size() == 1);
        CHECK(om->at(0) == X("xy", 2));
    }

    SECTION("lua insert")
    {
        vm.DoString("return om:insert(0, it('bar', 7))");
        REQUIRE(lua_gettop(vm) == 2);
        CHECK(vm.Get<bool>(-2));
        CHECK(vm.Get<int>(-1) == 0);
        lua_pop(vm, 2);

        vm.DoString("return om:insert(0, it('foo', 2))");
        REQUIRE(lua_gettop(vm) == 2);
        CHECK(vm.Get<bool>(-2));
        CHECK(vm.Get<int>(-1) == 0);
        lua_pop(vm, 2);

        vm.DoString("return om:insert(0, it('bar', 13))");
        REQUIRE(lua_gettop(vm) == 2);
        CHECK(!vm.Get<bool>(-2));
        CHECK(vm.Get<int>(-1) == 1);
        lua_pop(vm, 2);

        CHECK(om->size() == 2);
        CHECK(om->at(0) == X("foo", 2));
        CHECK(om->at(1) == X("bar", 7));
    }

    SECTION("populate")
    {
        om->emplace_back("abc", 7);
        om->emplace_back("xyz", -2);
        om->emplace_back("foo", 5);

        SECTION("get")
        {
            vm.DoString("return om[0]");
            REQUIRE(lua_gettop(vm) == 1);
            CHECK(vm.Get<X>(-1) == X("abc", 7));
            lua_pop(vm, 1);

            vm.DoString("return om[3]");
            REQUIRE(lua_gettop(vm) == 1);
            CHECK(lua_isnil(vm, -1));
            lua_pop(vm, 1);

            vm.DoString("return om.xyz");
            REQUIRE(lua_gettop(vm) == 1);
            CHECK(vm.Get<X>(-1) == X("xyz", -2));
            lua_pop(vm, 1);

            vm.DoString("return om.blahblah");
            REQUIRE(lua_gettop(vm) == 1);
            CHECK(lua_isnil(vm, -1));
            lua_pop(vm, 1);

            vm.DoString("return om[{}]");
            REQUIRE(lua_gettop(vm) == 1);
            CHECK(lua_isnil(vm, -1));
            lua_pop(vm, 1);
        }

        SECTION("erase")
        {
            vm.DoString("return om:erase(1)");
            REQUIRE(lua_gettop(vm) == 1);

            CHECK(vm.Get<size_t>(-1) == 1);
            CHECK(om->size() == 2);
            CHECK(om->at(0) == X("abc", 7));
            CHECK(om->at(1) == X("foo", 5));
            lua_pop(vm, 1);
        }

        SECTION("erase range")
        {
            vm.DoString("return om:erase(1, 3)");
            REQUIRE(lua_gettop(vm) == 1);

            CHECK(vm.Get<size_t>(-1) == 1);
            CHECK(om->size() == 1);
            CHECK(om->at(0) == X("abc", 7));
            lua_pop(vm, 1);
        }

        SECTION("remove")
        {
            vm.DoString("return om:remove(1)");
            REQUIRE(lua_gettop(vm) == 1);

            CHECK(vm.Get<X>(-1) == X("xyz", -2));
            CHECK(om->size() == 2);
            CHECK(om->at(0) == X("abc", 7));
            CHECK(om->at(1) == X("foo", 5));
            lua_pop(vm, 1);
        }

        SECTION("find")
        {
            vm.DoString("return om:find('xyz')");
            REQUIRE(lua_gettop(vm) == 2);
            CHECK(vm.Get<size_t>(-2) == 1);
            CHECK(vm.Get<X>(-1) == X("xyz", -2));
            lua_pop(vm, 2);

            vm.DoString("return om:find('not')");
            REQUIRE(lua_gettop(vm) == 1);
            CHECK(lua_isnil(vm, -1));
            lua_pop(vm, 1);
        }

        SECTION("to_table")
        {
            vm.DoString("                \n\
local t = om:to_table()                  \n\
assert(type(t) == 'table')               \n\
assert(t[0].k == 'abc' and t[0].v == 7)  \n\
assert(t[1].k == 'xyz' and t[1].v == -2) \n\
assert(t[2].k == 'foo' and t[2].v == 5)  \n\
assert(t[3] == nil)");
        }
    }
}
#endif

#include "container/ordered_map.lua.hpp"
NEPTOOLS_ORDERED_MAP_LUAGEN(om_item_test, Neptools::Test::OMItemTest,
                            Neptools::Test::OMItemTestTraits);
#include "ordered_map.binding.hpp"

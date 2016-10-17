#include "container/ordered_map.hpp"
#include <catch.hpp>

using namespace Neptools;

namespace
{
struct X : public OrderedMapItem
{
    X(int k, int v) : k{k}, v{v} { count += 2; }
    ~X() { --count; }
    X(const X&) = delete;
    void operator=(const X&) = delete;

    int k, v;
    static size_t count;
};
size_t X::count;

struct XTraits
{
    using type = int;
    const int& operator()(const X& x) { return x.k; }

    static void remove(OrderedMap<X, XTraits>&, X&)
    { --X::count; }
};

using OM = OrderedMap<X, XTraits>;
}

TEST_CASE("basic", "[ordered_map]")
{
    X::count = 0;
    {
        OM om;
        CHECK(om.empty());
        CHECK(om.size() == 0);
        om.emplace_back(2,2);
        om.emplace_back(1,7);
        CHECK(!om.empty());
        CHECK(om.size() == 2);

        SECTION("at")
        {
            CHECK(om.at(0).v == 2);
            CHECK(om.at(1).k == 1);
            CHECK_THROWS(om.at(2));
        }

        SECTION("operator[]")
        {
            CHECK(om[0].k == 2);
            CHECK(om[1].v == 7);
        }

        SECTION("front/back")
        {
            CHECK(om.front().v == 2);
            CHECK(om.back().v == 7);
        }

        SECTION("iterator")
        {
            auto it = om.begin();
            REQUIRE(it != om.end());
            CHECK(it->k == 2);
            ++it;
            REQUIRE(it != om.end());
            CHECK(it->k == 1);
            ++it;
            CHECK(it == om.end());
        }

        SECTION("reserve")
        {
            CHECK(om.capacity() >= 2);
            om.reserve(10);
            REQUIRE(om.size() == 2);
            CHECK(om.capacity() >= 10);

            CHECK(om[1].v == 7);
        }

        SECTION("clear")
        {
            om.clear();
            CHECK(om.empty());
        }

        SECTION("insert")
        {
            om.insert(om.begin()+1, MakeSmart<X>(9, 9));
            REQUIRE(om.size() == 3);
            CHECK(om[0].k == 2);
            CHECK(om[1].k == 9);
            CHECK(om[2].k == 1);
        }

        SECTION("insert existing")
        {
            om.insert(om.begin(), MakeSmart<X>(1, -1));
            REQUIRE(om.size() == 2);
            CHECK(om[0].v == 2);
            CHECK(om[1].v == 7);
        }

        SECTION("emplace")
        {
            om.emplace(om.begin(), 5, 5);
            REQUIRE(om.size() == 3);
            CHECK(om[0].k == 5);
            CHECK(om[1].k == 2);
            CHECK(om[2].k == 1);
        }

        SECTION("emplace existing")
        {
            om.emplace(om.begin(), 1, -1);
            REQUIRE(om.size() == 2);
            CHECK(om[0].v == 2);
            CHECK(om[1].v == 7);
        }

        SECTION("erase one")
        {
            om.erase(om.begin());
            REQUIRE(om.size() == 1);
            CHECK(om[0].k == 1);
        }

        SECTION("erase range")
        {
            om.erase(om.begin(), om.begin()+1);
            REQUIRE(om.size() == 1);
            CHECK(om[0].k == 1);
        }

        SECTION("erase everything")
        {
            om.erase(om.begin(), om.end());
            CHECK(om.empty());
        }

        SECTION("push_back")
        {
            om.push_back(MakeSmart<X>(3,3));
            REQUIRE(om.size() == 3);
            CHECK(om[0].k == 2);
            CHECK(om[1].k == 1);
            CHECK(om[2].k == 3);
        }

        SECTION("push_back existing")
        {
            om.push_back(MakeSmart<X>(1,77));
            REQUIRE(om.size() == 2);
            CHECK(om[0].v == 2);
            CHECK(om[1].v == 7);
        }

        SECTION("pop_back")
        {
            om.pop_back();
            REQUIRE(om.size() == 1);
            CHECK(om[0].v == 2);
            om.pop_back();
            REQUIRE(om.empty());
        }

        SECTION("nth")
        {
            CHECK(om.nth(0) == om.begin());
            CHECK(om.nth(0)->k == 2);
            CHECK(om.nth(1)->k == 1);
            CHECK(om.nth(2) == om.end());
            CHECK(om.index_of(om.nth(1)) == 1);
        }

        SECTION("map find")
        {
            CHECK(om.count(1) == 1);
            CHECK(om.count(3) == 0);
            CHECK(om.find(1)->k == 1);
            CHECK(om.find(3) == om.end());
        }

        SECTION("iterator_to")
        {
            X& x = om[1];
            CHECK(om.iterator_to(x) == om.nth(1));
        }

        SECTION("key_change")
        {
            om[0].k = 0;
            CHECK(om.count(0) == 0); // wrong rb-tree
            om.key_change(om.begin());
            CHECK(om.count(1) == 1); // fixed
        }
    }
    CHECK(X::count == 0);
}

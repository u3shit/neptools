#include "lua/function_ref.hpp"
#include "lua/type_traits.hpp"
#include <catch.hpp>

using namespace Neptools::Lua;

TEST_CASE("Lua FunctionRefs", "[Lua::FunctionRef]")
{
    State vm;
    vm.DoString("local i = 0 function f(j) i=i+1 return j or i end");
    REQUIRE(lua_getglobal(vm, "f") == LUA_TFUNCTION);

    SECTION("FunctionRef")
    {
        FunctionRef<> fr{lua_absindex(vm, -1)};

        CHECK(fr.Call<int>(vm) == 1);
        CHECK(fr.Call<int>(vm, 22) == 22);
        CHECK(fr.Call<int>(vm) == 3);
        CHECK(fr.Call<int>(vm, "10") == 10); // lua converts string to int

        CHECK_THROWS(fr.Call<int>(vm, "xx"));
    }

    SECTION("FunctionWrapGen")
    {
        FunctionWrapGen<int> fr{vm, lua_absindex(vm, -1)};

        CHECK(fr() == 1);
        CHECK(fr(77) == 77);

        CHECK_THROWS(fr("Hello"));
    }

    SECTION("FunctionWrap")
    {
        FunctionWrap<int()> fr{vm, lua_absindex(vm, -1)};

        CHECK(fr() == 1);
        static_assert(std::is_same_v<
            decltype(&FunctionWrap<int()>::operator()),
            int (FunctionWrap<int()>::*)()>);
    }
}


TEST_CASE("Lua::FunctionWrap for stl algorithm", "[Lua::FunctionRef]")
{
    State vm;
    vm.DoString("function f(a, b) return math.abs(a) < math.abs(b) end");
    REQUIRE(lua_getglobal(vm, "f") == LUA_TFUNCTION);

    std::vector<int> v{3, 9, -2, 7, -99, 13, -11};
    SECTION("FunctionWrap")
    {
        FunctionWrap<bool (int, int)> fr{vm, lua_absindex(vm, -1)};
        std::sort(v.begin(), v.end(), fr);
    }
    SECTION("FunctionWrapGen")
    {
        FunctionWrapGen<bool> fr{vm, lua_absindex(vm, -1)};
        std::sort(v.begin(), v.end(), fr);
    }

    std::vector<int> exp{-2, 3, 7, 9, -11, 13, -99};
    CHECK(v == exp);
}

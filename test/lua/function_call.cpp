#include "lua/function_call.hpp"
#include <catch.hpp>

using namespace Neptools::Lua;

int global;

void voidfun() { global = 1; }

#define FT(x) decltype(&x), &x
TEST_CASE("void function", "[lua]")
{
    State vm;
    vm.PushFunction<FT(voidfun)>();

    global = 0;
    lua_call(vm, 0, 0);
    CHECK(global == 1);
}

int intfun(int a, int b) { return a+b+3; }
TEST_CASE("int function", "[lua]")
{
    State vm;
    vm.PushFunction<FT(intfun)>();

    vm.Push(5);
    vm.Push(7);
    lua_call(vm, 2, 1);
    CHECK(vm.Get<int>() == 5+7+3);
    lua_pop(vm, 1);

    CHECK(lua_gettop(vm) == 0);
}

std::string strfun(size_t a, std::string b)
{
    std::string ret;
    for (size_t i = 0; i < a; ++i)
        ret += b;
    return ret;
}
TEST_CASE("string function", "[lua]")
{
    State vm;
    vm.PushFunction<FT(strfun)>();

    vm.Push(5);
    vm.Push("hello");
    lua_call(vm, 2, 1);
    CHECK(vm.Get<std::string>() == "hellohellohellohellohello");
    lua_pop(vm, 1);

    CHECK(lua_gettop(vm) == 0);
}

std::tuple<bool, int, std::string> tuplefun(bool a, int n)
{
    std::stringstream ss;
    ss << a << n;
    return std::make_tuple(a, n, ss.str());
}
TEST_CASE("tuple function", "[lua]")
{
    State vm;
    vm.PushFunction<FT(tuplefun)>();

    vm.Push(false);
    vm.Push(5);
    lua_call(vm, 2, 3);
    CHECK(vm.Get<bool>(1)        == false);
    CHECK(vm.Get<int>(2)         == 5);
    CHECK(vm.Get<std::string>(3) == "05");
    lua_pop(vm, 3);

    CHECK(lua_gettop(vm) == 0);
}

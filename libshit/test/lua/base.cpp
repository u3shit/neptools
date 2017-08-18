#include <libshit/lua/type_traits.hpp>
#include <catch.hpp>

using namespace Libshit::Lua;

TEST_CASE("state creation", "[lua]")
{
  State vm;
  vm.Catch([&]()
  {
    luaL_checkversion(vm);

    lua_pushinteger(vm, 123456);
    auto n = lua_tointeger(vm, -1);
    CHECK(n == 123456);
  });
}

TEST_CASE("catch lua error", "[lua]")
{
  State vm;
  try
  {
    vm.Catch([&]() { return luaL_error(vm, "foobar"); });
    FAIL("Expected Catch to throw an Error");
  }
  catch (const Error& e)
  {
    CHECK(std::string{e.what()} == "foobar");
  }
}

TEST_CASE("catch c++ error", "[lua]")
{
  State vm;
  try
  {
    vm.Catch([]() { throw std::runtime_error("zzfoo"); });
    FAIL("Expected Catch to throw an Error");
  }
  catch (const std::runtime_error& e)
  {
    CHECK(std::string{e.what()} == "zzfoo");
  }
}

TEST_CASE("random pushes", "[lua]")
{
  State vm;

  vm.Push(15);
  CHECK(lua_tointeger(vm, -1) == 15);
  lua_pop(vm, 1);

  vm.Push(3.1415);
  CHECK(lua_tonumber(vm, -1) == 3.1415);
  lua_pop(vm, 1);

  vm.Push("hello, world");
  CHECK(std::string{lua_tostring(vm, -1)} == "hello, world");
  lua_pop(vm, 1);

  vm.Push(true);
  CHECK(lua_isboolean(vm, -1));
  CHECK(lua_toboolean(vm, -1) == true);
  lua_pop(vm, 1);

  vm.Push<const char*>(nullptr);
  CHECK(lua_isnil(vm, -1));
  lua_pop(vm, 1);

  CHECK(lua_gettop(vm) == 0);
}

TEST_CASE("successful gets", "[lua]")
{
  State vm;

  lua_pushinteger(vm, 12);
  CHECK(vm.Get<int>() == 12);
  lua_pop(vm, 1);

  lua_pushnumber(vm, 2.7182);
  CHECK(vm.Get<double>() == 2.7182);
  lua_pop(vm, 1);

  lua_pushliteral(vm, "foo");
  CHECK(std::string(vm.Get<const char*>()) == "foo");
  CHECK(vm.Get<std::string>() == "foo");
  lua_pop(vm, 1);

  lua_pushboolean(vm, 0);
  CHECK(vm.Get<bool>() == false);
  lua_pop(vm, 1);

  CHECK(lua_gettop(vm) == 0);
}

TEST_CASE("optional vals", "[lua]")
{
  State vm;

  lua_pushinteger(vm, 19);
  auto opt = vm.Opt<int>(1);
  REQUIRE(opt);
  CHECK(*opt == 19);
  lua_pop(vm, 1);

  REQUIRE(lua_isnone(vm, 1));
  opt = vm.Opt<int>(1);
  CHECK_FALSE(opt);

  lua_pushnil(vm);
  opt = vm.Opt<int>(1);
  CHECK_FALSE(opt);
  lua_pop(vm, 1);

  CHECK(lua_gettop(vm) == 0);
}

TEST_CASE("fail get", "[lua]")
{
  State vm;

  lua_pushboolean(vm, true);
  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Get<int>(); }),
                    Catch::Matchers::Contains(
                      "invalid lua value: integer expected, got boolean"));
  lua_settop(vm, 1); // apparently luaL_error leave some junk on the stack

  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Get<double>(); }),
                    Catch::Matchers::Contains(
                      "invalid lua value: number expected, got boolean"));
  lua_settop(vm, 1);

  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Get<const char*>(); }),
                    Catch::Matchers::Contains(
                      "invalid lua value: string expected, got boolean"));
  lua_settop(vm, 1);
  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Get<std::string>(); }),
                    Catch::Matchers::Contains(
                      "invalid lua value: string expected, got boolean"));
  lua_settop(vm, 0);

  lua_pushinteger(vm, 77);
  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Get<bool>(); }),
                    Catch::Matchers::Contains(
                      "invalid lua value: boolean expected, got number"));
}

TEST_CASE("fail check", "[lua]")
{
  State vm;

  lua_pushboolean(vm, true);
  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Check<int>(1); }),
                    Catch::Matchers::Contains(
                      "bad argument #1 to '?' (integer expected, got boolean)"));
  lua_settop(vm, 1); // apparently luaL_error leave some junk on the stack

  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Check<double>(1); }),
                    Catch::Matchers::Contains(
                      "bad argument #1 to '?' (number expected, got boolean)"));
  lua_settop(vm, 1);

  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Check<const char*>(1); }),
                    Catch::Matchers::Contains(
                      "bad argument #1 to '?' (string expected, got boolean)"));
  lua_settop(vm, 1);
  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Check<std::string>(1); }),
                    Catch::Matchers::Contains(
                      "bad argument #1 to '?' (string expected, got boolean)"));
  lua_settop(vm, 0);

  lua_pushinteger(vm, 77);
  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Check<bool>(1); }),
                    Catch::Matchers::Contains(
                      "bad argument #1 to '?' (boolean expected, got number)"));
}

TEST_CASE("opt correct", "[lua]")
{
  State vm;

  lua_pushinteger(vm, 72);
  auto opt = vm.Opt<int>(1);
  REQUIRE(opt);
  CHECK(*opt == 72);
  lua_pop(vm, 1);

  CHECK(lua_gettop(vm) == 0);
}

TEST_CASE("opt nil/none", "[lua]")
{
  State vm;

  auto opt = vm.Opt<int>(1);
  CHECK(!opt);

  lua_pushnil(vm);
  opt = vm.Opt<int>(1);
  CHECK(!opt);
  lua_pop(vm, 1);

  CHECK(lua_gettop(vm) == 0);
}

TEST_CASE("opt invalid", "[lua]")
{
  State vm;

  lua_pushliteral(vm, "foo");
  CHECK_THROWS_WITH(vm.Catch([&]() { vm.Opt<int>(1); }),
                    Catch::Matchers::Contains(
                      "bad argument #1 to '?' (integer expected, got string)"));
}

namespace
{
  static int global;
  struct DestructorTest
  {
    ~DestructorTest() { global = 17; }
  };
}

TEST_CASE("exception interoperability", "[lua]")
{
  State vm;
  global = 0;

  CHECK_THROWS_WITH(
    vm.Catch([&]() { DestructorTest tst; luaL_error(vm, "foobaz"); }),
    Catch::Matchers::Contains("foobaz"));
  CHECK(global == 17); // destructor didn't run if failed
}

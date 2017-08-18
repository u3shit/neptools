#include <libshit/lua/user_type.hpp>
#include <libshit/lua/dynamic_object.hpp>
#include <libshit/lua/function_call.hpp>
#include <catch.hpp>

#define FT(x) decltype(&x), &x

using namespace Libshit;
using namespace Libshit::Lua;

static int global;

struct Smart final : public SmartObject
{
  int x = 0;

  LIBSHIT_LUA_CLASS;
};

struct Foo final : public RefCounted, public DynamicObject
{
  int local_var = 0;
  LIBSHIT_LUAGEN(get="::Libshit::Lua::GetRefCountedOwnedMember")
  Smart smart;
  void DoIt(int x) { local_var = x; }

  Foo() = default; // no ctor generated otherwise.. (bug?)
  ~Foo() { global += 13; }

  LIBSHIT_DYNAMIC_OBJECT;
};

namespace Bar
{
  namespace Baz
  {
    struct Asdfgh final : public DynamicObject
    {
      Asdfgh() = default;
      LIBSHIT_DYNAMIC_OBJECT;
    };
  }
}

struct Baz : public DynamicObject
{
  Baz() = default;
  void SetGlobal(int val) { global = val; }
  int GetRandom() { return 4; }

  LIBSHIT_DYNAMIC_OBJECT;
};

TEST_CASE("shared check memory", "[lua]")
{
  {
    State vm;

    global = 0;
    const char* str;
    SECTION("normal") str = "local x = foo.new()";
    SECTION("short-cut") str = "local x = foo()";
    SECTION("explicit call") str = "local x = foo():__gc()";

    vm.DoString(str);
  }
  CHECK(global == 13);
}

TEST_CASE("resurrect shared object", "[lua]")
{
  {
    State vm;

    global = 0;
    auto ptr = MakeSmart<Foo>();
    vm.Push(ptr);
    lua_setglobal(vm, "fooobj");

    vm.DoString("fooobj:__gc() assert(getmetatable(fooobj) == nil)");
    REQUIRE(global == 0);

    vm.Push(ptr);
    lua_setglobal(vm, "fooobj");
    vm.DoString("fooobj:do_it(123)");
    CHECK(global == 0);
    CHECK(ptr->local_var == 123);
  }
  CHECK(global == 13);
}

TEST_CASE("member function without helpers", "[lua]")
{
  State vm;

  REQUIRE(luaL_loadstring(vm, "local x = foo() x:do_it(77) return x") == 0);
  lua_call(vm, 0, 1);
  CHECK(vm.Get<Foo>().local_var == 77);
}

TEST_CASE("member function with helpers", "[lua]")
{
  State vm;

  const char* str;
  int val;
  SECTION("normal call") { str = "baz():set_global(42)"; val = 42; }
  SECTION("sugar") { str = "baz().global = 43"; val = 43; }
  SECTION("read") { str = "local x = baz() x.global = x.random"; val = 4; }

  vm.DoString(str);
  CHECK(global == val);
}

TEST_CASE("field access", "[lua]")
{
  State vm;
  auto ptr = MakeSmart<Foo>();
  vm.Push(ptr);
  lua_setglobal(vm, "foo");
  ptr->local_var = 13;

  SECTION("get")
  {
    const char* str;
    SECTION("plain") { str = "return foo:get_local_var()"; }
    SECTION("sugar") { str = "return foo.local_var"; }
    vm.DoString(str);
    CHECK(vm.Get<int>() == 13);
  }

  SECTION("set")
  {
    const char* str;
    SECTION("plain") { str = "foo:set_local_var(42)"; }
    SECTION("sugar") { str = "foo.local_var = 42"; }
    vm.DoString(str);
    CHECK(ptr->local_var == 42);
  }
}

TEST_CASE("invalid field access yields nil", "[lua]")
{
  State vm;
  vm.DoString("return foo().bar");
  CHECK(lua_isnil(vm, -1));
}

TEST_CASE("dotted type name", "[lua]")
{
  State vm;
  vm.DoString("local x = bar.baz.asdfgh()");
}

TEST_CASE("aliased objects", "[lua]")
{
  State vm;
  vm.DoString(
    "local f = foo()\n"
    "assert(f ~= f.smart and f.smart == f.smart)\n"
    "f.smart.x = 7\n"
    "assert(f.smart.x == 7)\n");
}

struct A : public DynamicObject
{
  int x = 0;

  LIBSHIT_DYNAMIC_OBJECT;
};

struct B : public DynamicObject
{
  int y = 1;

  LIBSHIT_DYNAMIC_OBJECT;
};

struct Multi : public A, public B
{
  Multi() = default;
  SharedPtr<B> ptr;

  LIBSHIT_DYNAMIC_OBJECT;
};

static DynamicObject& GetDynamicObject(Multi& m) { return static_cast<A&>(m); }

TEST_CASE("multiple inheritance", "[lua]")
{
  State vm;
  vm.DoString(R"(
local m = multi()
m.ptr = m
m.ptr.y = 13
assert(m.x == 0, "m.x")
assert(m.y == 13, "m.y")
)");
}


#include "user_type.binding.hpp"

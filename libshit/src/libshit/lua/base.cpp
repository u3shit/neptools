#include "type_traits.hpp"
#include "../assert.hpp"
#include "libshit/lua/base_funcs.lua.h"

#include <cstring>

namespace Libshit::Lua
{

  char reftbl;

  State::State(int) : StateRef{luaL_newstate()}
  {
    if (!vm) LIBSHIT_THROW(std::bad_alloc{});
  }

  // todo: do we need it?
  static int panic(lua_State* vm)
  {
    size_t len;
    auto msg = lua_tolstring(vm, -1, &len);
    if (msg) LIBSHIT_THROW(Error{{msg, len}});
    else LIBSHIT_THROW(Error{"Lua PANIC"});
  }

  const char* StateRef::TypeName(int idx)
  {
    LIBSHIT_LUA_GETTOP(vm, top);
    if (!luaL_getmetafield(vm, idx, "__name")) // 0/+1
    {
      LIBSHIT_LUA_CHECKTOP(vm, top);
      return luaL_typename(vm, idx);
    }

    auto ret = lua_tostring(vm, -1); // +1
    LIBSHIT_ASSERT_MSG(ret, "invalid __name");
    lua_pop(vm, 1); // 0
    LIBSHIT_LUA_CHECKTOP(vm, top);
    return ret;
  }

  State::State() : State(0)
  {
    Catch(
      [](StateRef vm)
      {
        LIBSHIT_LUA_GETTOP(vm, top);

        lua_atpanic(vm, panic);
        luaL_openlibs(vm);

        // init reftable
        lua_newtable(vm);               // +1
        lua_createtable(vm, 0, 1);      // +2 metatbl
        lua_pushliteral(vm, "v");       // +3
        lua_setfield(vm, -2, "__mode"); // +2
        lua_setmetatable(vm, -2);       // +1
        lua_rawsetp(vm, LUA_REGISTRYINDEX, &reftbl); // 0

        // helper funs
        LIBSHIT_LUA_RUNBC(vm, base_funcs, 0);

        for (auto r : Registers())
          r(vm);

        LIBSHIT_LUA_CHECKTOP(vm, top);
      }, *this);
  }

  State::~State()
  {
    if (vm) lua_close(vm);
  }

#ifdef _MSC_VER
  int StateRef::SEHFilter(lua_State* vm, unsigned code)
  {
    if ((code & 0xffffff00) != 0xe24c4a00)
      return EXCEPTION_CONTINUE_SEARCH;
    if (lua_gettop(vm) == 0 || !lua_isstring(vm, -1))
      return EXCEPTION_CONTINUE_SEARCH;

    error_msg = lua_tolstring(vm, -1, &error_len);
    LIBSHIT_ASSERT(error_msg);
    return EXCEPTION_EXECUTE_HANDLER;
  }
#else
  void StateRef::HandleDotdotdotCatch()
  {
    if (lua_gettop(vm) == 0 || !lua_isstring(vm, -1)) throw;

    error_msg = lua_tolstring(vm, -1, &error_len);
    if (strcmp(error_msg, "C++ exception") == 0) throw;
  }
#endif

  void StateRef::TypeError(bool arg, const char* expected, int idx)
  {
    const char* actual = TypeName(idx);
    std::stringstream ss;
    ss << expected << " expected, got " << actual;

    GetError(arg, idx, ss.str().c_str());
  }

  void StateRef::GetError(bool arg, int idx, const char* msg)
  {
    if (arg) luaL_argerror(vm, idx, msg);
    else luaL_error(vm, "invalid lua value: %s", msg);
    LIBSHIT_UNREACHABLE("lua_error returned");
  }

  StateRef::RawLen01Ret StateRef::RawLen01(int idx)
  {
    LIBSHIT_LUA_GETTOP(vm, top);
    LIBSHIT_ASSERT(lua_type(vm, idx) == LUA_TTABLE);
    auto len = lua_rawlen(vm, idx);
    auto type = lua_rawgeti(vm, idx, 0); // +1
    lua_pop(vm, 1); // 0
    LIBSHIT_LUA_CHECKTOP(vm, top);
    return {len + !IsNoneOrNil(type), IsNoneOrNil(type)};
  }

  std::pair<size_t, int> StateRef::Ipairs01Prep(int idx)
  {
    size_t i = 0;
    int type;
    if (IsNoneOrNil(type = lua_rawgeti(vm, idx, i))) // +1
    {
      lua_pop(vm, 1); // 0
      type = lua_rawgeti(vm, idx, ++i); // +1
    }
    return {i, type};
  }

  size_t StateRef::Unpack01(int idx)
  {
    LIBSHIT_LUA_GETTOP(vm, top);
    LIBSHIT_ASSERT(idx > 0);

    auto [len,one] = RawLen01(idx);
    if (len > INT_MAX || !lua_checkstack(vm, len))
      luaL_error(vm, "too many items to unpack");

    for (size_t i = 0; i < len; ++i)
      lua_rawgeti(vm, idx, i + one);

    LIBSHIT_LUA_CHECKTOP(vm, int(top+len));
    return len;
  }


  void StateRef::SetRecTable(const char* name, int idx)
  {
    LIBSHIT_LUA_GETTOP(vm, top);

    const char* dot;
    while (dot = strchr(name, '.'))
    {
      // tbl = tbl[name_chunk] ||= {}
      // {
      // will be pushed again when subtable doesn't exists, but optimize for
      // common case where it already exists
      lua_pushlstring(vm, name, dot-name); // +1
      auto typ = lua_rawget(vm, -2); // +1
      if (IsNoneOrNil(typ)) // no subtable, create it
      {
        lua_pop(vm, 1); // 0

        lua_createtable(vm, 0, 1); // +1 new tbl
        lua_pushlstring(vm, name, dot-name); // +2
        lua_pushvalue(vm, -2); // +3
        lua_rawset(vm, -4); // +1
      }

      lua_remove(vm, -2); // 0
      // }
      name = dot+1;
    }

    // tbl[name] = value
    lua_pushvalue(vm, idx); // +1
    lua_setfield(vm, -2, name); // 0
    lua_pop(vm, 1); // -1

    LIBSHIT_LUA_CHECKTOP(vm, top-1);
  }

  void StateRef::DoString(const char* str)
  {
    if (luaL_dostring(vm, str))
      LIBSHIT_THROW(std::runtime_error{lua_tostring(vm, -1)});
  }

  thread_local const char* StateRef::error_msg;
  thread_local size_t StateRef::error_len;

}

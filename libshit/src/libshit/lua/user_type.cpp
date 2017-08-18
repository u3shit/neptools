#include "user_type.hpp"
#include <cstring>

namespace Libshit::Lua
{

  TypeBuilder::TypeBuilder(StateRef vm, const char* name, bool instantiable)
    : vm{vm}, instantiable{instantiable}
  {
    LIBSHIT_LUA_GETTOP(vm, top);

    // type table
    lua_createtable(vm, 0, 0); // +1
    lua_pushvalue(vm, -1); // +2
    lua_rawsetp(vm, LUA_REGISTRYINDEX, name+1); // +1

    if (instantiable)
    {
      lua_getfield(vm, LUA_REGISTRYINDEX, "libshit_new_mt"); // +2
      LIBSHIT_ASSERT(lua_istable(vm, -1));
      lua_setmetatable(vm, -2); // +1

      // metatable
      lua_createtable(vm, 0, 5); // +2
      lua_pushliteral(vm, "private"); // +3
      lua_setfield(vm, -2, "__metatable"); // +2

      lua_pushvalue(vm, -1); // +3
      lua_setfield(vm, -2, "__index"); // +2

      lua_pushinteger(vm, 0); // +3
      lua_rawsetp(vm, -2, name); // +2

      lua_pushvalue(vm, -1); // +3
      lua_rawsetp(vm, LUA_REGISTRYINDEX, name); //+2

      // metatable.__name = name
      lua_pushstring(vm, name); // +3
      lua_setfield(vm, -2, "__name"); // +2

      // is function
      lua_pushlightuserdata(vm, const_cast<char*>(name)); // +3
      lua_pushcclosure(vm, &IsFunc, 1); // +3
      SetField("is"); // +2
    }
    else
    {
      // "metatable"
      lua_pushvalue(vm, -1); //+2
    }

    // set global name
    lua_pushglobaltable(vm); // +3
    vm.SetRecTable(name, -3); // +2

    LIBSHIT_LUA_CHECKTOP(vm, top+2);
  }

  void TypeBuilder::TaggedNew()
  {
    LIBSHIT_LUA_GETTOP(vm, top);
    auto t = lua_getfield(vm, LUA_REGISTRYINDEX, "libshit_tagged_new_mt"); // +1
    LIBSHIT_ASSERT(t == LUA_TTABLE); (void) t;
    lua_setmetatable(vm, -3); // 0
    LIBSHIT_LUA_CHECKTOP(vm, top);
  }

  static void SetMt(StateRef vm, const char* dst, const char* mt)
  {
    lua_getfield(vm, LUA_REGISTRYINDEX, mt);
    LIBSHIT_ASSERT(lua_isfunction(vm, -1));
    lua_pushvalue(vm, -2); // +2
    lua_call(vm, 1, 1); // +1
    lua_setfield(vm, -2, dst); // +0
  }

  void TypeBuilder::Done()
  {
    LIBSHIT_LUA_GETTOP(vm, top);

    if (instantiable)
    {
      bool has_get_ = false, has_get = false,
        has_set_ = false, has_set = false;

      lua_pushnil(vm); // +1
      while (lua_next(vm, -2)) // +2/0
      {
        lua_pop(vm, 1); // +1
        if (lua_type(vm, -1) != LUA_TSTRING) continue;

        auto name = lua_tostring(vm, -1);
        if (strcmp(name, "get") == 0)      has_get  = true;
        if (strncmp(name, "get_", 4) == 0) has_get_ = true;
        if (strcmp(name, "set") == 0)      has_set  = true;
        if (strncmp(name, "set_", 4) == 0) has_set_ = true;

        if (has_get_ && has_set_)
        {
          lua_pop(vm, 1); // 0
          break;
        }
      }


      if (has_get_ || has_get)
        SetMt(
          vm, "__index",
          has_get_ ? "libshit_mt_index" : "libshit_mt_index_light");

      if (has_set_) SetMt(vm, "__newindex", "libshit_mt_newindex");
      else if (has_set)
      {
        lua_getfield(vm, -1, "set"); // +1
        LIBSHIT_ASSERT(lua_isfunction(vm, -1));
        lua_setfield(vm, -2, "__newindex"); // +0
      }
    }

    lua_remove(vm, -2);

    LIBSHIT_LUA_CHECKTOP(vm, top-1);
  }

  void TypeBuilder::DoInherit(ptrdiff_t offs)
  {
    LIBSHIT_LUA_GETTOP(vm, top);
    // -1: base class meta
    // -2: this meta

    LIBSHIT_ASSERT(instantiable);
    LIBSHIT_ASSERT(lua_type(vm, -1) == LUA_TTABLE);
    LIBSHIT_ASSERT(lua_type(vm, -2) == LUA_TTABLE);

    // for k,v in pairs(base_mt) do
    lua_pushnil(vm); // +1
    while (lua_next(vm, -2)) // +2/0
    {
      // if not this_mt[k] then
      lua_pushvalue(vm, -2); // +3
      auto type = lua_rawget(vm, -5); // +3
      if (IsNoneOrNil(type))
      {
        lua_pop(vm, 1); // +2
        // this_mt[k] = if_below ? v+offs : v
        lua_pushvalue(vm, -2); //+3

        // if type(k) == "lightuserdata" and type(v) == "number"
        if (lua_islightuserdata(vm, -3) && lua_type(vm, -2) == LUA_TNUMBER)
          lua_pushinteger(vm, lua_tointeger(vm, -2)+offs); // +4
        else
          lua_pushvalue(vm, -2); //+4
        lua_rawset(vm, -6); // +2
        lua_pop(vm, 1); // +1
      }
      else
        lua_pop(vm, 2); // +1
    }
    lua_pop(vm, 1); // -1

    LIBSHIT_LUA_CHECKTOP(vm, top-1);
  }

  int TypeBuilder::IsFunc(lua_State* vm) noexcept
  {
    if (!lua_getmetatable(vm, 1))
    {
      lua_pushboolean(vm, false);
      return 1;
    }

    lua_pushvalue(vm, lua_upvalueindex(1));
    LIBSHIT_ASSERT(lua_islightuserdata(vm, -1));

    auto type = lua_rawget(vm, -2);
    lua_pushboolean(vm, type == LUA_TNUMBER);
    return 1;
  }

}

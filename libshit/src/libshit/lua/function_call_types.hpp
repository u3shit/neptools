#ifndef UUID_D230EBCE_94C2_4E8F_B2FD_99237D511676
#define UUID_D230EBCE_94C2_4E8F_B2FD_99237D511676
#pragma once

#ifndef LIBSHIT_WITHOUT_LUA
#  include <lua.hpp>
#endif

#include "../assert.hpp"

namespace Libshit::Lua
{

  class StateRef;

  // placeholder to skip parsing this argument
  struct Skip {};

  // placeholder to indicate that the function takes extra arguments
  struct VarArg {};

  // the function pushes result manually
  struct RetNum
  {
    RetNum(int n) : n{n} {}
    int n;
  };

  // like skip, but stores index
  class Any
  {
  public:
    constexpr Any(int idx) : idx{idx}
    { LIBSHIT_ASSERT_MSG(idx > 0, "Index must be absolute"); }

    constexpr int Get() const { return idx; }
    constexpr operator int() const { return idx; }

  private:
    int idx;
  };

#ifndef LIBSHIT_WITHOUT_LUA
  // ensure type but do not parse
  template <int Type>
  class Raw : public Any
  {
  public:
    static_assert(
      Type == LUA_TNIL || Type == LUA_TNUMBER || Type == LUA_TBOOLEAN ||
      Type == LUA_TSTRING || Type == LUA_TTABLE || Type == LUA_TFUNCTION ||
      Type == LUA_TUSERDATA || Type == LUA_TTHREAD || Type == LUA_TLIGHTUSERDATA,
      "Type is not a lua type constant");

    static constexpr int TYPE = Type;
    using Any::Any;
  };

  using RawString = Raw<LUA_TSTRING>;
  using RawTable = Raw<LUA_TTABLE>;
#endif

  template <typename T, typename Enable = void> struct TupleLike;

}

#endif

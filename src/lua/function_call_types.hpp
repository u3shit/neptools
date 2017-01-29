#ifndef UUID_D230EBCE_94C2_4E8F_B2FD_99237D511676
#define UUID_D230EBCE_94C2_4E8F_B2FD_99237D511676
#pragma once

#include <lua.hpp>

namespace Neptools::Lua
{

// placeholder to skip parsing this argument
struct Skip {};

// the function pushes result manually
struct RetNum
{
    RetNum(int n) : n{n} {}
    int n;
};

// ensure type but do not parse
template <int Type>
struct Raw
{
    static_assert(
        Type == LUA_TNIL || Type == LUA_TNUMBER || Type == LUA_TBOOLEAN ||
        Type == LUA_TSTRING || Type == LUA_TTABLE || Type == LUA_TFUNCTION ||
        Type == LUA_TUSERDATA || Type == LUA_TTHREAD || Type == LUA_TLIGHTUSERDATA,
        "Type is not a lua type constant");
    static constexpr int TYPE = Type;
};

}

#endif

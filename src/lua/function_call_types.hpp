#ifndef UUID_D230EBCE_94C2_4E8F_B2FD_99237D511676
#define UUID_D230EBCE_94C2_4E8F_B2FD_99237D511676
#pragma once

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

}

#endif

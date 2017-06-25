#ifndef UUID_C93BD681_48D9_4438_8FBF_15DBED6015BB
#define UUID_C93BD681_48D9_4438_8FBF_15DBED6015BB
#pragma once

#include "type_traits.hpp"

namespace Neptools::Lua
{

struct StaticClass {};

template<typename T>
struct TypeTraits<T, std::enable_if_t<std::is_base_of_v<StaticClass, T>>>
{
    constexpr static bool TYPE_TAGGED = false;
};

template<typename T>
struct UserTypeTraits<T, std::enable_if_t<std::is_base_of_v<StaticClass, T>>>
{
    inline static void MetatableCreate(StateRef) {}
    static constexpr bool NEEDS_GC = false;
};

}

#endif

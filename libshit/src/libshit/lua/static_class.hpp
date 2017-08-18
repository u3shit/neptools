#ifndef UUID_C93BD681_48D9_4438_8FBF_15DBED6015BB
#define UUID_C93BD681_48D9_4438_8FBF_15DBED6015BB
#pragma once

#include "type_traits.hpp"
#include "../meta.hpp"

namespace Libshit::Lua
{

  struct LIBSHIT_LUAGEN(no_inherit=true) StaticClass {};

#ifndef LIBSHIT_WITHOUT_LUA

  template<typename T>
  struct TypeTraits<T, std::enable_if_t<std::is_base_of_v<StaticClass, T>>>
  {
    static void PrintName(std::ostream& os) { os << TYPE_NAME<T>; }
  };

  template<typename T>
  struct UserTypeTraits<T, std::enable_if_t<std::is_base_of_v<StaticClass, T>>>
  {
    constexpr static bool INSTANTIABLE = false;
    static constexpr bool NEEDS_GC = false;
  };

#endif

}

#endif

#ifndef UUID_4EA5C16A_B4BE_478D_82C5_12ED8E4F72FD
#define UUID_4EA5C16A_B4BE_478D_82C5_12ED8E4F72FD
#pragma once

namespace Libshit::Lua
{

  class TypeBuilder;

  template <typename T, typename Enable = void> struct TypeRegisterTraits
  {
    static void Register(TypeBuilder& bld); // so you can fully specialize
  };

}

#endif

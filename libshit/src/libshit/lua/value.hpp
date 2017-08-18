#ifndef UUID_610B8988_C51F_4145_9579_4503FF5A294B
#define UUID_610B8988_C51F_4145_9579_4503FF5A294B
#pragma once

#include "base.hpp"

namespace Libshit::Lua
{

  class StackValue
  {
  public:
    static constexpr const bool HAS_VM = false;

    StackValue(int idx) noexcept : idx{idx} { LIBSHIT_ASSERT(idx > 0); }
    StackValue(StateRef, int idx) noexcept : StackValue{idx} {}

    void Push(StateRef vm) const { lua_pushvalue(vm, idx); }

    void Destroy(StateRef) noexcept {}
  private:
    int idx;
  };

}

#endif

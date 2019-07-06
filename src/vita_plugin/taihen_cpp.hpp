#ifndef GUARD_CRANIALLY_GENEVESE_DISOMY_DISCLOUTS_4156
#define GUARD_CRANIALLY_GENEVESE_DISOMY_DISCLOUTS_4156
#pragma once

#include <libshit/except.hpp>

#include <utility>
#include <taihen.h>

namespace Neptools::VitaPlugin
{

  // replacement for tai_hook_ref_t and TAI_CONTINUE that works in c++
  template <typename F> class TaiHook;
  template <typename Res, typename... Args>
  class TaiHook<Res(Args...)>
  {
  public:
    Res operator()(Args... args)
    {
      return (n->next ? n->next->func : n->old)(std::forward<Args>(args)...);
    };

    operator uintptr_t*() { return reinterpret_cast<uintptr_t*>(&n); }
    operator uintptr_t() { return reinterpret_cast<uintptr_t>(n); }

  private:
    using FuncPtr = Res(*)(Args...);
    struct Node
    {
      Node* next;
      FuncPtr func, old;
    };
    Node* n;
  };

  inline static const char* TAI_MAIN_MOD_STR =
    reinterpret_cast<const char*>(TAI_MAIN_MODULE);

  LIBSHIT_GEN_EXCEPTION_TYPE(TaiError, std::runtime_error);

}

#endif
